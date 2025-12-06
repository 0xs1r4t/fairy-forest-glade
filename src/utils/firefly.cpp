#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
using namespace std;

#include "firefly.h"

Firefly::Firefly(int count, const glm::vec3 &centerPos, float radius)
    : count(count), radius(radius)
{

    cout << "Creating " << count << " fireflies..." << endl;

    positions.reserve(count);
    velocities.reserve(count);
    colors.reserve(count);
    phases.reserve(count);
    speeds.reserve(count);
    sizes.reserve(count);

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> radiusDist(0.0f, radius);
    uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    uniform_real_distribution<float> heightDist(-2.0f, 4.0f);
    uniform_real_distribution<float> phaseDist(0.0f, 2.0f * 3.14159f);
    uniform_real_distribution<float> speedDist(0.5f, 1.5f);
    uniform_real_distribution<float> sizeDist(0.08f, 0.15f); // Varied sizes
    uniform_real_distribution<float> hueDist(0.0f, 1.0f);

    for (int i = 0; i < count; i++)
    {
        // Random spawn position
        float r = radiusDist(gen);
        float angle = angleDist(gen);
        float height = heightDist(gen);

        glm::vec3 pos = centerPos + glm::vec3(
                                        r * cos(angle),
                                        height,
                                        r * sin(angle));

        positions.push_back(pos);
        velocities.push_back(glm::vec3(0.0f));
        phases.push_back(phaseDist(gen));
        speeds.push_back(speedDist(gen));
        sizes.push_back(sizeDist(gen));

        // Rainbow of warm colors
        float hue = hueDist(gen);
        glm::vec3 color;

        if (hue < 0.4f)
        {
            // Warm yellow-orange fireflies (most common)
            float warmth = hue / 0.4f;
            color = glm::vec3(1.0f, 0.6f + warmth * 0.3f, 0.1f);
        }
        else if (hue < 0.65f)
        {
            // Green-cyan fireflies
            float greenness = (hue - 0.4f) / 0.25f;
            color = glm::vec3(0.2f, 1.0f, 0.3f + greenness * 0.4f);
        }
        else if (hue < 0.85f)
        {
            // Blue fireflies
            float blueness = (hue - 0.65f) / 0.2f;
            color = glm::vec3(0.2f + blueness * 0.3f, 0.5f, 1.0f);
        }
        else
        {
            // Pink-purple fireflies (rare)
            float pinkness = (hue - 0.85f) / 0.15f;
            color = glm::vec3(1.0f, 0.2f, 0.6f + pinkness * 0.3f);
        }

        // Make colors MORE saturated
        color = glm::normalize(color) * 0.9f; // Normalize then scale

        colors.push_back(color);
    }

    setupMesh();
    cout << "Fireflies created!" << endl;
}

Firefly::~Firefly()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteBuffers(1, &colorVBO);
    glDeleteBuffers(1, &sizeVBO);
}

void Firefly::setupMesh()
{
    // Simple billboard quad (2 triangles)
    float quadVertices[] = {
        // positions
        -0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.5f, 0.5f, 0.0f};

    unsigned int indices[] = {
        0, 1, 2,
        0, 2, 3};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Quad vertices
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    // Instance position buffer
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glVertexAttribDivisor(1, 1);

    // Instance color buffer
    glGenBuffers(1, &colorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glVertexAttribDivisor(2, 1);

    // Instance size buffer
    glGenBuffers(1, &sizeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizes.size() * sizeof(float), sizes.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);
}

void Firefly::Update(float deltaTime, const glm::vec3 &fairyPos)
{
    float time = glfwGetTime();

    for (int i = 0; i < count; i++)
    {
        // Organic circular movement pattern (like ShaderToy)
        float phase = phases[i] + time * speeds[i];

        // Smooth figure-8 / lissajous curves
        glm::vec3 movement;
        movement.x = cos(phase * 1.3f) * 2.0f;
        movement.y = sin(phase * 0.7f) * 1.5f;
        movement.z = sin(phase * 1.1f) * 2.0f;

        // Gentle drift toward fairy
        glm::vec3 toFairy = fairyPos - positions[i];
        float distToFairy = glm::length(toFairy);

        if (distToFairy > 0.5f)
        {
            toFairy = glm::normalize(toFairy);
            velocities[i] += toFairy * 0.5f * deltaTime;
        }

        // Combine movement
        velocities[i] = movement * 0.3f;

        // Apply damping
        velocities[i] *= 0.92f;

        // Update position
        positions[i] += velocities[i] * deltaTime;

        // Keep within bounds around fairy
        glm::vec3 offset = positions[i] - fairyPos;
        float dist = glm::length(offset);
        if (dist > radius)
        {
            positions[i] = fairyPos + glm::normalize(offset) * radius;
        }

        // Clamp height
        if (positions[i].y < fairyPos.y - 3.0f)
        {
            positions[i].y = fairyPos.y - 3.0f;
        }
        if (positions[i].y > fairyPos.y + 5.0f)
        {
            positions[i].y = fairyPos.y + 5.0f;
        }
    }

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());
}

void Firefly::Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection)
{
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setFloat("time", (float)glfwGetTime());

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, count);
    glBindVertexArray(0);
}
