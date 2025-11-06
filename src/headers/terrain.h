#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.h"
#include "perlin.h"

class Terrain
{
public:
    // Terrain parameters
    int width;
    int depth;
    float scale;
    float heightScale;

    // OpenGL buffers
    unsigned int VAO, VBO, EBO;

    // Mesh data
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Constructor
    Terrain(int width = 100, int depth = 100, float scale = 0.1f, float heightScale = 10.0f, unsigned int seed = 0);

    // Destructor
    ~Terrain();

    // Generate terrain using Perlin noise
    void generate(unsigned int seed = 0);

    // Render the terrain
    void Draw(Shader &shader);

private:
    PerlinNoise perlin;

    // Setup OpenGL buffers
    void setupMesh();

    // Get height at a specific position using Perlin noise
    float getHeight(float x, float z);

    // Calculate normal for a vertex
    glm::vec3 calculateNormal(int x, int z);
};