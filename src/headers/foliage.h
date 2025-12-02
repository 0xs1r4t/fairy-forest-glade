#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
using namespace std;

#include "shader.h"
#include "terrain.h"
#include "camera.h"

enum class FoliageType
{
    GRASS,
    FLOWER
};

class Foliage
{
public:
    // Constructor
    Foliage(Terrain *terrain, FoliageType type, int count, float height, float width);

    // Destructor
    ~Foliage();

    // Draw with frustum culling
    void Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
              const Camera::Frustum &frustum, const Camera &camera);

    // Public members
    vector<glm::vec3> positions;
    FoliageType type;

private:
    Terrain *terrain;
    unsigned int VAO, VBO, EBO;
    unsigned int instanceVBO;
    int count;
    float height;
    float width;
    float boundingRadius;

    vector<glm::vec3> visiblePositions;

    // Setup methods
    void generatePositions();
    void setupCrossQuad();
};