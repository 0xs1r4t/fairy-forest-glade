#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
using namespace std;

#include "foliage.h"

Foliage::Foliage(Terrain* terrain, FoliageType type, int count, float height, float width)
    : terrain(terrain), type(type), count(count), height(height), width(width) {
    
    string typeName;
    switch(type) {
        case FoliageType::GRASS:
            typeName = "grass";
            boundingRadius = 0.5f;
            break;
        case FoliageType::FLOWER:
            typeName = "flower";
            boundingRadius = 0.7f;
            break;
        case FoliageType::TREE:
            typeName = "tree";
            boundingRadius = 3.0f;
            break;
    }
    
    cout << "Generating " << typeName << " positions..." << endl;
    
    // Pre-allocate memory
    positions.reserve(count);
    visiblePositions.reserve(count / 2);
    
    // Generate positions on terrain
    generatePositions();
    
    cout << "Placed " << positions.size() << " " << typeName << " instances" << endl;
    
    // Setup cross-quad geometry
    setupCrossQuad();
    
    // Setup instance buffer
    glGenBuffers(1, &instanceVBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

Foliage::~Foliage() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &instanceVBO);
}

void Foliage::generatePositions() {
    // Use different seed offsets for different foliage types
    static unsigned int seedOffset = 0;
    srand(static_cast<unsigned int>(time(0)) + seedOffset++);
    
    int attempts = count * 2;
    for (int i = 0; i < attempts && static_cast<int>(positions.size()) < count; i++) {
        float x = ((float)rand() / RAND_MAX) * (terrain->width * terrain->scale) - 
                  (terrain->width * terrain->scale) / 2.0f;
        float z = ((float)rand() / RAND_MAX) * (terrain->height * terrain->scale) - 
                  (terrain->height * terrain->scale) / 2.0f;
        
        float y = terrain->getHeight(x, z);
        glm::vec3 normal = terrain->getNormal(x, z);
        
        // Different placement criteria based on type
        bool validPlacement = false;
        
        switch(type) {
            case FoliageType::GRASS:
                // Grass grows almost anywhere flat
                validPlacement = (normal.y > 0.5f && y > -6.0f);
                break;
                
            case FoliageType::FLOWER:
                // Flowers prefer flatter, slightly elevated areas
                validPlacement = (normal.y > 0.7f && y > -4.0f && y < 3.0f);
                break;
                
            case FoliageType::TREE:
                // Trees need very flat ground and avoid peaks/valleys
                validPlacement = (normal.y > 0.85f && y > -2.0f && y < 2.0f);
                break;
        }
        
        if (validPlacement) {
            positions.push_back(glm::vec3(x, y, z));
        }
    }
}

void Foliage::setupCrossQuad() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // SINGLE QUAD (not cross-quad) - will be rotated by billboard shader
    // Centered at origin, extends in X (width) and Y (height)
    
    float halfWidth = width / 2.0f;
    
    // 4 corners of a single quad
    // Position (XYZ), Normal (XYZ), TexCoord (UV)
    vertices = {
        // Bottom left
        -halfWidth, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
        // Bottom right  
        halfWidth, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
        // Top right
        halfWidth, height, 0.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
        // Top left
        -halfWidth, height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
    };
    
    // Two triangles forming the quad
    indices = {
        0, 1, 2,  // First triangle
        0, 2, 3   // Second triangle
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    
    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    
    // TexCoord attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    
    glBindVertexArray(0);
}

void Foliage::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                   const Camera::Frustum& frustum, const Camera& camera) {
    visiblePositions.clear();
    
    // Frustum culling - THIS RUNS EVERY FRAME
    for (const auto& pos : positions) {
        if (camera.IsSphereInFrustum(frustum, pos, boundingRadius)) {
            visiblePositions.push_back(pos);
        }
    }
    
    // Debug output every 60 frames
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        std::string typeName;
        switch(type) {
            case FoliageType::GRASS: typeName = "Grass"; break;
            case FoliageType::FLOWER: typeName = "Flowers"; break;
            case FoliageType::TREE: typeName = "Trees"; break;
        }
        std::cout << typeName << ": Rendering " << visiblePositions.size() 
                  << " / " << positions.size() << " instances" << std::endl;
    }
    
    if (visiblePositions.empty()) return;
    
    // Update instance buffer EVERY FRAME
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, visiblePositions.size() * sizeof(glm::vec3),
                 visiblePositions.data(), GL_DYNAMIC_DRAW);
    
    // IMPORTANT: Bind VAO and draw
    // Shader uniforms are already set in main.cpp before calling Draw()
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 
                           static_cast<GLsizei>(visiblePositions.size()));
    glBindVertexArray(0);
}