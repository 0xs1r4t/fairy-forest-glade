#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
using namespace std;

#include "shader.h"

struct TerrainVertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec3 colour;
};

class Terrain
{
public:
    // terrain params
    int height;
    int width;
    int scale;

    Terrain(int height, int width, float scale = 1.0f, float heightScale = 1.0f);
    ~Terrain();

    void drawTerrain(Shader &shader, const glm::mat4 &view);
    float getHeight(float x, float z);
    glm::vec3 getNormal(float x, float z);
    void regenerateTerrain(int octaves, float frequency, float amplitude);

private:
    void generateTerrain();
    void calculateNormals();
    void setupMesh();

    // terrain params
    float heightScale;
    int octaves = 6;
    float frequency = 0.05f;

    vector<TerrainVertex> vertices;
    vector<unsigned int> indices;
    vector<float> heightMap;
    unsigned int VAO, VBO, EBO;
};