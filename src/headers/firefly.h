#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <random>
using namespace std;

#include "shader.h"
#include "camera.h"

class Firefly
{
public:
    Firefly(int count, const glm::vec3 &centerPos, float radius);
    ~Firefly();

    void Update(float deltaTime, const glm::vec3 &fairyPos);
    void Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);

    vector<glm::vec3> GetPositions() const { return positions; }
    vector<glm::vec3> GetColors() const { return colors; }

private:
    int count;
    float radius;

    vector<glm::vec3> positions;
    vector<glm::vec3> velocities;
    vector<glm::vec3> colors;
    vector<float> phases;
    vector<float> speeds; // Individual movement speeds
    vector<float> sizes;  // Individual firefly sizes

    unsigned int VAO, VBO, instanceVBO, colorVBO, sizeVBO;

    void setupMesh();
};
