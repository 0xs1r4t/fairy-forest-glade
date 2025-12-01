#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <vector>
#include <random>
using namespace std;

#include "shader.h"
#include "camera.h"

class Firefly {
public:
    Firefly(int count, const glm::vec3& centerPos, float radius);
    ~Firefly();
    
    // Update firefly positions and animations
    void Update(float deltaTime, const glm::vec3& fairyPos);
    
    // Draw all fireflies
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection);
    
    // Get firefly positions for lighting calculations
    vector<glm::vec3> GetPositions() const { return positions; }
    vector<glm::vec3> GetColors() const { return colors; }
    
private:
    int count;
    float radius; // Area around fairy where fireflies hover
    
    vector<glm::vec3> positions;
    vector<glm::vec3> velocities;
    vector<glm::vec3> colors;
    vector<float> phases; // For pulsing glow effect
    
    // OpenGL buffers
    unsigned int VAO, VBO, instanceVBO, colorVBO;
    
    void setupMesh();
    void generateIcosphere(vector<float>& vertices, vector<unsigned int>& indices);
};