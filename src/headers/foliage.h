#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
using namespace std;

#include "shader.h"
#include "terrain.h"
#include "camera.h"
#include "lod.h"

enum class FoliageType
{
    GRASS,
    FLOWER
};

class Foliage
{
public:
    // Constructor
    Foliage(Terrain *terrain, FoliageType type, int count, float height, float width,
            const LODConfig &lodConfig = LODConfig());

    // Destructor
    ~Foliage();

    // Draw with frustum culling
    void Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
              const Camera::Frustum &frustum, const Camera &camera);

    // get visible grass count
    int GetVisibleCount() const { return visiblePositions.size(); }

    // Public members
    vector<glm::vec3> positions;
    FoliageType type;

private:
    Terrain *terrain;
    LODConfig lodConfig;
    unsigned int VAO, VBO, EBO;
    unsigned int instanceVBO;
    unsigned int textureIndexVBO;

    int count;
    float height;
    float width;
    float boundingRadius;
    float terrainHeightScale; // store max terrain height for placement

    vector<glm::vec3> visiblePositions;
    std::vector<float> textureIndices;
    std::vector<float> visibleTextureIndices;

    // Setup methods
    void generatePositions();
    void setupCrossQuad();
};