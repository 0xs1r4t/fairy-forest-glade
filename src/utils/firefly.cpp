#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
using namespace std;

#include "firefly.h"

Firefly::Firefly(int count, const glm::vec3& centerPos, float radius)
    : count(count), radius(radius) {
    
    cout << "Creating " << count << " fireflies..." << endl;
    
    positions.reserve(count);
    velocities.reserve(count);
    colors.reserve(count);
    phases.reserve(count);
    
    // Random number generation
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> radiusDist(0.0f, radius);
    uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    uniform_real_distribution<float> heightDist(-2.0f, 4.0f);
    uniform_real_distribution<float> phaseDist(0.0f, 2.0f * 3.14159f);
    
    // Initialize fireflies in random positions around center
    for (int i = 0; i < count; i++) {
        float r = radiusDist(gen);
        float angle = angleDist(gen);
        float height = heightDist(gen);
        
        glm::vec3 pos = centerPos + glm::vec3(
            r * cos(angle),
            height,
            r * sin(angle)
        );
        
        positions.push_back(pos);
        velocities.push_back(glm::vec3(0.0f)); // Start stationary
        
        // Warm yellow-orange glow colors with variation
        float colorVariation = (rand() % 100) / 100.0f;
        glm::vec3 color = glm::vec3(
            1.0f,                           // Red
            0.8f + colorVariation * 0.2f,   // Green (yellowish)
            0.3f + colorVariation * 0.2f    // Blue (warm)
        );
        colors.push_back(color);
        
        phases.push_back(phaseDist(gen)); // Random starting phase for pulsing
    }
    
    setupMesh();
    
    cout << "Fireflies created!" << endl;
}

Firefly::~Firefly() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteBuffers(1, &colorVBO);
}

void Firefly::generateIcosphere(vector<float>& vertices, vector<unsigned int>& indices) {
    // Simple low-poly sphere (icosahedron subdivided once)
    // For fireflies, we want VERY simple geometry
    
    const float t = (1.0f + sqrt(5.0f)) / 2.0f;
    const float scale = 0.02f; // scale for rendering tiny fireflies
    
    // 12 vertices of icosahedron
    vector<glm::vec3> verts = {
        glm::normalize(glm::vec3(-1,  t,  0)) * scale,
        glm::normalize(glm::vec3( 1,  t,  0)) * scale,
        glm::normalize(glm::vec3(-1, -t,  0)) * scale,
        glm::normalize(glm::vec3( 1, -t,  0)) * scale,
        
        glm::normalize(glm::vec3( 0, -1,  t)) * scale,
        glm::normalize(glm::vec3( 0,  1,  t)) * scale,
        glm::normalize(glm::vec3( 0, -1, -t)) * scale,
        glm::normalize(glm::vec3( 0,  1, -t)) * scale,
        
        glm::normalize(glm::vec3( t,  0, -1)) * scale,
        glm::normalize(glm::vec3( t,  0,  1)) * scale,
        glm::normalize(glm::vec3(-t,  0, -1)) * scale,
        glm::normalize(glm::vec3(-t,  0,  1)) * scale
    };
    
    // Convert to flat array
    for (const auto& v : verts) {
        vertices.push_back(v.x);
        vertices.push_back(v.y);
        vertices.push_back(v.z);
    }
    
    // 20 triangular faces
    vector<unsigned int> faces = {
        0, 11, 5,   0, 5, 1,    0, 1, 7,    0, 7, 10,   0, 10, 11,
        1, 5, 9,    5, 11, 4,   11, 10, 2,  10, 7, 6,   7, 1, 8,
        3, 9, 4,    3, 4, 2,    3, 2, 6,    3, 6, 8,    3, 8, 9,
        4, 9, 5,    2, 4, 11,   6, 2, 10,   8, 6, 7,    9, 8, 1
    };
    
    indices = faces;
}

void Firefly::setupMesh() {
    vector<float> vertices;
    vector<unsigned int> indices;
    
    generateIcosphere(vertices, indices);
    
    // Create VAO, VBO for sphere geometry
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // Instance position buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(1, 1);
    
    // Instance color buffer
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glVertexAttribDivisor(2, 1);
    
    glBindVertexArray(0);
}

void Firefly::Update(float deltaTime, const glm::vec3& fairyPos) {
    const float followSpeed = 1.5f;
    const float wanderSpeed = 0.5f;
    const float damping = 0.95f;
    
    for (int i = 0; i < count; i++) {
        // Move toward fairy with some randomness
        glm::vec3 toFairy = fairyPos - positions[i];
        float distToFairy = glm::length(toFairy);
        
        // Don't get too close or too far
        if (distToFairy > 0.1f) {
            toFairy = glm::normalize(toFairy);
            
            // Gentle attraction to fairy
            velocities[i] += toFairy * followSpeed * deltaTime;
            
            // Add wandering motion (Perlin-like)
            float time = phases[i] + glfwGetTime() * 2.0f;
            glm::vec3 wander(
                sin(time * 1.3f) * wanderSpeed,
                sin(time * 0.7f) * wanderSpeed,
                cos(time * 1.1f) * wanderSpeed
            );
            velocities[i] += wander * deltaTime;
        }
        
        // Apply damping
        velocities[i] *= damping;
        
        // Update position
        positions[i] += velocities[i] * deltaTime;
        
        // Keep within reasonable height
        if (positions[i].y < fairyPos.y - 3.0f) {
            positions[i].y = fairyPos.y - 3.0f;
            velocities[i].y = abs(velocities[i].y) * 0.5f;
        }
        if (positions[i].y > fairyPos.y + 5.0f) {
            positions[i].y = fairyPos.y + 5.0f;
            velocities[i].y = -abs(velocities[i].y) * 0.5f;
        }
        
        // Update pulsing phase
        phases[i] += deltaTime;
    }
    
    // Update instance buffer with new positions
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
}

void Firefly::Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection) {
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    // Pass time for pulsing glow effect
    shader.setFloat("time", (float)glfwGetTime());
    
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0, count); // 20 faces * 3 vertices
    glBindVertexArray(0);
}