#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
using namespace std;

#include "shader.h"
#include "terrain.h"
#include "model.h"
#include "camera.h"

class Flowers
{
public:
    Flowers(Terrain *terrain, Model *flowerModel, int count);

    void Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
              const Camera::Frustum &frustum, const Camera &camera);
    vector<glm::vec3> positions;

private:
    Terrain *terrain;
    Model *flowerModel;
    unsigned int instanceVBO;
    unsigned int visibleInstanceVBO;
    int count;
    vector<glm::vec3> visiblePositions;
};