#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <random>
using namespace std;

#include "shader.h"
#include "model.h"

struct LeafCluster
{
    glm::vec3 attachPoint;   // Position on branch where leaves cluster
    glm::vec3 emitterNormal; // For lighting gradient (from sphere/cube)
    float radius;            // Cluster size
    int leafCount;           // How many leaves in this cluster
};

struct LeafInstance
{
    glm::vec3 offset;       // Relative to cluster center
    glm::vec3 customNormal; // Transferred from emitter for shading
    float scale;            // Size variation
    int textureIndex;       // Which leaf texture (0-3)
};

class TreeFoliage
{
public:
    TreeFoliage(const char *branchModelPath);
    ~TreeFoliage();

    void GenerateLeafClusters(int clustersPerBranch = 8, int leavesPerCluster = 15);
    void Draw(Shader &leafShader, Shader &branchShader,
              const glm::mat4 &model, const glm::mat4 &view,
              const glm::mat4 &projection, const glm::vec3 &cameraPos);

    void LoadLeafTextures(const vector<const char *> &texturePaths);

private:
    // Branch mesh
    Model *branchModel;
    vector<glm::vec3> branchVertices; // Extract for attachment points

    // Leaf data
    vector<LeafCluster> clusters;
    vector<LeafInstance> allLeaves;
    vector<unsigned int> leafTextures;

    // OpenGL buffers
    unsigned int quadVAO, quadVBO, quadEBO;
    unsigned int instanceVBO; // For leaf instance data

    void setupQuadMesh();
    void extractBranchVertices();
    void transferNormals(const glm::vec3 &clusterCenter, LeafInstance &leaf);
};
