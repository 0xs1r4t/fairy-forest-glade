#include <cstdlib>
#include <ctime>
#include <iostream>
using namespace std;

#include "flowers.h"
#include "noise.h"

Flowers::Flowers(Terrain *terrain, Model *flowerModel, int count)
    : terrain(terrain), flowerModel(flowerModel), count(count)
{
    cout << "\n=== FLOWERS DEBUG ===" << endl;
    cout << "Placing flower instances..." << endl;

    // Seed random for better distribution
    srand(static_cast<unsigned int>(time(0)) + 12345);

    // Terrain info
    cout << "Terrain dimensions: " << terrain->width << "x" << terrain->height << endl;
    cout << "Terrain scale: " << terrain->scale << endl;

    // Generate random flower positions
    int attempts = count * 2;
    for (int i = 0; i < attempts && positions.size() < count; i++)
    {
        float x = ((float)rand() / RAND_MAX) * (terrain->width * terrain->scale) - (terrain->width * terrain->scale) / 2.0f;
        float z = ((float)rand() / RAND_MAX) * (terrain->height * terrain->scale) - (terrain->height * terrain->scale) / 2.0f;

        float y = terrain->getHeight(x, z);
        glm::vec3 normal = terrain->getNormal(x, z);

        // More lenient placement conditions
        if (normal.y > 0.5f && y > -6.0f)
        {
            // NO offset - place exactly at terrain height
            positions.push_back(glm::vec3(x, y, z));

            // Debug first few positions
            if (positions.size() <= 3)
            {
                cout << "Flower #" << positions.size() << " at: ("
                     << x << ", " << y << ", " << z << ")" << endl;
            }
        }
    }

    cout << "Placed " << positions.size() << " flower instances out of " << count << " requested" << endl;

    if (positions.size() > 0)
    {
        cout << "First flower position: (" << positions[0].x << ", " << positions[0].y << ", " << positions[0].z << ")" << endl;
        cout << "Last flower position: (" << positions.back().x << ", " << positions.back().y << ", " << positions.back().z << ")" << endl;
    }

    // Create VBO for visible instances
    glGenBuffers(1, &visibleInstanceVBO);

    // Attach instance attribute to each mesh
    for (Mesh &mesh : flowerModel->meshes)
    {
        glBindVertexArray(mesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, visibleInstanceVBO);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glVertexAttribDivisor(4, 1);
        glBindVertexArray(0);
    }

    cout << "===================\n"
         << endl;
}

void Flowers::Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
                   const Camera::Frustum &frustum, const Camera &camera)
{
    static int frameCount = 0;
    frameCount++;

    // Clear visible positions
    visiblePositions.clear();

    // Frustum culling with bounding sphere
    // Flower models may be slightly larger than grass
    float boundingRadius = 1.5f; // Adjust based on your flower model size

    for (const auto &pos : positions)
    {
        // Check if this flower instance is within the frustum
        if (camera.IsSphereInFrustum(frustum, pos, boundingRadius))
        {
            visiblePositions.push_back(pos);
        }
    }

    // Debug output every 60 frames
    if (frameCount % 60 == 0)
    {
        cout << "Flowers: Rendering " << visiblePositions.size() << " / "
             << positions.size() << " instances" << endl;
    }

    // If nothing visible, skip rendering
    if (visiblePositions.empty())
    {
        return;
    }

    // Update visible instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, visibleInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, visiblePositions.size() * sizeof(glm::vec3),
                 &visiblePositions[0], GL_DYNAMIC_DRAW);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);

    // Simple model matrix - NO transforms, identity only
    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);

    // Draw all flower meshes
    for (Mesh &mesh : flowerModel->meshes)
    {
        glBindVertexArray(mesh.VAO);
        glDrawElementsInstanced(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0,
                                visiblePositions.size());
        glBindVertexArray(0);
    }
}