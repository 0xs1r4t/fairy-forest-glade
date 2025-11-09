#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
using namespace std;

class Terrain
{
public:
    Terrain() {}
    ~Terrain() {}

    void generateTerrain(int width, int height);
    void renderTerrain();
    void setHeightMap(const vector<float> &heightMap);

private:
    unsigned int terrainVAO;
};