#include <glm/glm.hpp>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
using namespace std;

#include "foliage.h"

Foliage::Foliage(Terrain *terrain, FoliageType type, int count, float height, float width,
                 const LODConfig &lodConfig)
    : terrain(terrain), type(type), count(count), height(height), width(width), lodConfig(lodConfig)
{

    string typeName;
    switch (type)
    {
    case FoliageType::GRASS:
        typeName = "grass";
        boundingRadius = 0.5f;
        break;
    case FoliageType::FLOWER:
        typeName = "flower";
        boundingRadius = 0.7f;
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
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

Foliage::~Foliage()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &instanceVBO);
}

void Foliage::generatePositions()
{
    // Use different seed offsets for different foliage types
    static unsigned int seedOffset = 0;
    srand(static_cast<unsigned int>(time(0)) + seedOffset++);

    int attempts = count * 2;
    for (int i = 0; i < attempts && static_cast<int>(positions.size()) < count; i++)
    {
        float x = (float(rand()) / RAND_MAX) * terrain->width * terrain->scale - (terrain->width * terrain->scale / 2.0f);
        float z = (float(rand()) / RAND_MAX) * terrain->height * terrain->scale - (terrain->height * terrain->scale / 2.0f);
        float y = terrain->getHeight(x, z);
        glm::vec3 normal = terrain->getNormal(x, z);

        bool validPlacement = false;
        switch (type)
        {
        case FoliageType::GRASS:
            // Grass grows almost anywhere flat
            validPlacement = (normal.y > 0.5f && y > -3.0f); // <- CHANGED (removed upper limit)
            break;

        case FoliageType::FLOWER:
            // Flowers prefer flatter, mid-elevation areas
            validPlacement = (normal.y > 0.7f && y > 0.0f && y < 7.0f); // <- CHANGED (was -4 to 3)
            break;
        }

        if (validPlacement)
            positions.push_back(glm::vec3(x, y, z));
    }
}

void Foliage::setupCrossQuad()
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // SINGLE QUAD (not cross-quad) - will be rotated by billboard shader
    // Centered at origin, extends in X (width) and Y (height)

    float halfWidth = width / 2.0f;

    // 4 corners of a single quad
    // Position (XYZ), Normal (XYZ), TexCoord (UV)
    vertices = {
        // Bottom left
        -halfWidth, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        // Bottom right
        halfWidth, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        // Top right
        halfWidth, height, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        // Top left
        -halfWidth, height, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f};

    // Two triangles forming the quad
    indices = {
        0, 1, 2, // First triangle
        0, 2, 3  // Second triangle
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

    // Normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));

    // TexCoord attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    glBindVertexArray(0);
}

void Foliage::Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection,
                   const Camera::Frustum &frustum, const Camera &camera)
{
    visiblePositions.clear();

    for (const auto &pos : positions)
    {
        // Frustum culling
        if (!camera.IsSphereInFrustum(frustum, pos, boundingRadius))
        {
            continue;
        }

        // Calculate distance
        float distance = glm::distance(camera.Position, pos);

        // LOD culling
        if (distance > lodConfig.farDistance)
            continue;

        // DENSITY SAMPLING with ULTRA-NEAR zone
        float densityThreshold;

        if (type == FoliageType::GRASS)
        {
            // GRASS: Ultra-dense within 8m, then normal LOD
            if (distance < 8.0f)
            {
                densityThreshold = 1.0f; // 100% density carpet
            }
            else if (distance < lodConfig.nearDistance)
            {
                // Smooth transition from ultra-near to near
                float t = (distance - 8.0f) / (lodConfig.nearDistance - 8.0f);
                densityThreshold = glm::mix(1.0f, lodConfig.nearDensity, t);
            }
            else
            {
                densityThreshold = lodConfig.GetDensityMultiplier(distance);
            }
        }
        else
        {
            // Other foliage uses normal LOD
            densityThreshold = lodConfig.GetDensityMultiplier(distance);
        }

        // Deterministic hash for stable sampling
        unsigned int hash = (unsigned int)(pos.x * 73856093) ^
                            (unsigned int)(pos.z * 19349663);
        float random = (hash % 1000) / 1000.0f;

        if (random < densityThreshold)
        {
            visiblePositions.push_back(pos);
        }
    }

    // Debug output
    static int frameCount = 0;
    if (++frameCount % 60 == 0)
    {
        std::string typeName = (type == FoliageType::GRASS) ? "Grass" : "Flowers";
        std::cout << typeName << ": Rendering " << visiblePositions.size()
                  << " / " << positions.size() << " instances (LOD active)" << std::endl;
    }

    if (visiblePositions.empty())
        return;

    // Update instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, visiblePositions.size() * sizeof(glm::vec3),
                 visiblePositions.data(), GL_DYNAMIC_DRAW);

    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0,
                            static_cast<GLsizei>(visiblePositions.size()));
    glBindVertexArray(0);
}
