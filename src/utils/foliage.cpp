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
    vector<float> vertices;
    vector<unsigned int> indices;
    
    auto addQuad = [&](float angleY, unsigned int baseIdx) {
        float c = cos(glm::radians(angleY));
        float s = sin(glm::radians(angleY));
        glm::vec3 normal(s, 0, c);
        
        float x1 = -width/2 * c;
        float z1 = -width/2 * s;
        float x2 = width/2 * c;
        float z2 = width/2 * s;
        
        // Pos, Normal, TexCoord (8 floats per vertex)
        vertices.insert(vertices.end(), {x1, 0.0f, z1, normal.x, normal.y, normal.z, 0.0f, 0.0f});
        vertices.insert(vertices.end(), {x2, 0.0f, z2, normal.x, normal.y, normal.z, 1.0f, 0.0f});
        vertices.insert(vertices.end(), {x2, height, z2, normal.x, normal.y, normal.z, 1.0f, 1.0f});
        vertices.insert(vertices.end(), {x1, height, z1, normal.x, normal.y, normal.z, 0.0f, 1.0f});
        
        indices.push_back(baseIdx + 0u);
        indices.push_back(baseIdx + 1u);
        indices.push_back(baseIdx + 2u);
        indices.push_back(baseIdx + 0u);
        indices.push_back(baseIdx + 2u);
        indices.push_back(baseIdx + 3u);
    };
    
    // 3 cross quads for all foliage types
    addQuad(0.0f, 0u);
    addQuad(60.0f, 4u);
    addQuad(120.0f, 8u);
    
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
    
    // Frustum culling
    for (const auto& pos : positions) {
        if (camera.IsSphereInFrustum(frustum, pos, boundingRadius)) {
            visiblePositions.push_back(pos);
        }
    }
    
    // Debug output every 60 frames
    static int frameCount = 0;
    if (frameCount++ % 60 == 0) {
        string typeName;
        switch(type) {
            case FoliageType::GRASS: typeName = "Grass"; break;
            case FoliageType::FLOWER: typeName = "Flowers"; break;
            case FoliageType::TREE: typeName = "Trees"; break;
        }
        cout << typeName << ": Rendering " << visiblePositions.size() 
                  << " / " << positions.size() << " instances" << endl;
    }
    
    if (visiblePositions.empty()) return;
    
    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, visiblePositions.size() * sizeof(glm::vec3),
                 visiblePositions.data(), GL_DYNAMIC_DRAW);
    
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(visiblePositions.size()));
    glBindVertexArray(0);
}