#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;

#include "grass.h"
#include "noise.h"

Grass::Grass(Terrain *terrain, Model *grassModel, int count)
    : terrain(terrain), grassModel(grassModel), count(count)
{
    cout << "Placing grass instances..." << endl;

    // Seed random for better distribution
    srand(static_cast<unsigned int>(time(0)));

    // Generate random grass positions
    int attempts = count * 2; // Try more positions to account for rejections
    for (int i = 0; i < attempts && positions.size() < count; i++)
    {
        // Random position within terrain bounds
        float x = ((float)rand() / RAND_MAX) * (terrain->width * terrain->scale) - (terrain->width * terrain->scale) / 2.0f;
        float z = ((float)rand() / RAND_MAX) * (terrain->height * terrain->scale) - (terrain->height * terrain->scale) / 2.0f;

        float y = terrain->getHeight(x, z);
        glm::vec3 normal = terrain->getNormal(x, z);

        // Only place grass on relatively flat terrain
        if (normal.y > 0.65f && y > -4.5f) // More lenient slope threshold
        {
            positions.push_back(glm::vec3(x, y - 5.0f, z)); // -5 to match terrain offset
        }
    }

    cout << "Placed " << positions.size() << " grass instances out of " << count << " requested" << endl;

    // Instance VBO for position offsets
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), &positions[0], GL_STATIC_DRAW);

    // Attach instance attribute to each mesh of the grass model
    for (Mesh &mesh : grassModel->meshes)
    {
        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glEnableVertexAttribArray(4); // location 4 for instance offset
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glVertexAttribDivisor(4, 1); // one offset per instance
        glBindVertexArray(0);
    }
}

void Grass::Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection)
{
    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    glm::mat4 model = glm::mat4(1.0f);
    // BIGGER GRASS: Scale up from 0.5 to 1.5
    model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));
    shader.setMat4("model", model);

    for (Mesh &mesh : grassModel->meshes)
    {
        glBindVertexArray(mesh.VAO);
        glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0, positions.size());
        glBindVertexArray(0);
    }
}