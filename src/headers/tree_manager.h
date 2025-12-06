#pragma once

#include <vector>
#include <random>
using namespace std;

#include "tree_foliage.h"
#include "terrain.h"
#include "camera.h"
#include "shader.h"
#include "lod.h"

struct TreeInstance
{
    glm::vec3 position;
    float scale;
    float rotation;
    bool useThickType;
    float boundingRadius;
};

class TreeManager
{
public:
    TreeManager(Terrain *terrain, TreeFoliage *normalType, TreeFoliage *thickType,
                int count, const LODConfig &lodConfig,
                glm::vec3 exclusionCenter, float exclusionRadius);

    void Draw(Shader &leafShader, Shader &branchShader,
              const glm::mat4 &view, const glm::mat4 &projection,
              const Camera::Frustum &frustum, const Camera &camera);

    int GetVisibleCount() const { return visibleCount; }
    int GetTotalCount() const { return trees.size(); }

private:
    Terrain *terrain;
    TreeFoliage *normalTree;
    TreeFoliage *thickTree;
    LODConfig lodConfig;
    vector<TreeInstance> trees;
    int visibleCount;

    void generateTreePositions(int count, glm::vec3 exclusionCenter, float exclusionRadius);
};
