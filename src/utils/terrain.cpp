#include "terrain.h"
#include "noise.h"
#include <iostream>

Terrain::Terrain(int width, int height, float scale, float heightScale)
    : width(width), height(height), scale(scale), heightScale(heightScale)
{
    generateTerrain();
    setupMesh();
}

Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Terrain::generateTerrain()
{
    vertices.clear();
    indices.clear();
    heightMap.clear();
    heightMap.resize(width * height);

    // Generate height map using FBM
    for (int z = 0; z < height; z++)
    {
        for (int x = 0; x < width; x++)
        {
            float xPos = x * scale;
            float zPos = z * scale;

            // Use FBM for natural-looking terrain
            glm::vec2 samplePos = glm::vec2(x, z) * frequency;
            float heightValue = Noise::warpedFBM(samplePos, octaves);

            // Add some ridges for variety
            float ridges = Noise::ridgedNoise(samplePos * 0.5f, 4);
            heightValue = glm::mix(heightValue, ridges, 0.3f);

            float yPos = heightValue * heightScale;
            heightMap[z * width + x] = yPos;

            // Create vertex
            TerrainVertex vertex;
            vertex.position = glm::vec3(xPos - (width * scale) / 2.0f, yPos, zPos - (height * scale) / 2.0f);
            vertex.texCoords = glm::vec2((float)x / width, (float)z / height);

            // Procedural coloring based on height
            if (yPos < heightScale * 0.2f)
            {
                // Low areas - grass
                vertex.colour = glm::vec3(0.2f, 0.6f, 0.2f);
            }
            else if (yPos < heightScale * 0.5f)
            {
                // Mid areas - forest
                vertex.colour = glm::vec3(0.15f, 0.4f, 0.15f);
            }
            else if (yPos < heightScale * 0.75f)
            {
                // High areas - rocky
                vertex.colour = glm::vec3(0.5f, 0.4f, 0.3f);
            }
            else
            {
                // Peaks - snow
                vertex.colour = glm::vec3(0.9f, 0.9f, 0.95f);
            }

            vertices.push_back(vertex);
        }
    }

    // Generate indices for triangle strip
    for (int z = 0; z < height - 1; z++)
    {
        for (int x = 0; x < width - 1; x++)
        {
            int topLeft = z * width + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * width + x;
            int bottomRight = bottomLeft + 1;

            // First triangle
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            // Second triangle
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    calculateNormals();

    std::cout << "Terrain generated: " << width << "x" << height
              << " (" << vertices.size() << " vertices, "
              << indices.size() / 3 << " triangles)" << std::endl;
}

void Terrain::calculateNormals()
{
    // Initialize all normals to zero
    for (auto &vertex : vertices)
    {
        vertex.normal = glm::vec3(0.0f);
    }

    // Calculate face normals and accumulate
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        glm::vec3 v0 = vertices[i0].position;
        glm::vec3 v1 = vertices[i1].position;
        glm::vec3 v2 = vertices[i2].position;

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        vertices[i0].normal += normal;
        vertices[i1].normal += normal;
        vertices[i2].normal += normal;
    }

    // Normalize all vertex normals
    for (auto &vertex : vertices)
    {
        vertex.normal = glm::normalize(vertex.normal);
    }
}

void Terrain::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(TerrainVertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void *)0);

    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void *)offsetof(TerrainVertex, normal));

    // TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void *)offsetof(TerrainVertex, texCoords));

    // Color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(TerrainVertex), (void *)offsetof(TerrainVertex, colour));

    glBindVertexArray(0);
}

void Terrain::drawTerrain(Shader &shader, const glm::mat4 &model)
{
    shader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

float Terrain::getHeight(float x, float z)
{
    // Convert world coordinates to grid coordinates
    float gridX = (x + (width * scale) / 2.0f) / scale;
    float gridZ = (z + (height * scale) / 2.0f) / scale;

    // Clamp to terrain bounds
    if (gridX < 0 || gridX >= width - 1 || gridZ < 0 || gridZ >= height - 1)
        return 0.0f;

    // Bilinear interpolation
    int x0 = (int)gridX;
    int z0 = (int)gridZ;
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    float fx = gridX - x0;
    float fz = gridZ - z0;

    float h00 = heightMap[z0 * width + x0];
    float h10 = heightMap[z0 * width + x1];
    float h01 = heightMap[z1 * width + x0];
    float h11 = heightMap[z1 * width + x1];

    float h0 = glm::mix(h00, h10, fx);
    float h1 = glm::mix(h01, h11, fx);

    return glm::mix(h0, h1, fz);
}

glm::vec3 Terrain::getNormal(float x, float z)
{
    // Sample heights around the point
    float offset = scale * 0.1f;
    float hL = getHeight(x - offset, z);
    float hR = getHeight(x + offset, z);
    float hD = getHeight(x, z - offset);
    float hU = getHeight(x, z + offset);

    // Calculate normal from height differences
    glm::vec3 normal;
    normal.x = hL - hR;
    normal.y = 2.0f * offset;
    normal.z = hD - hU;

    return glm::normalize(normal);
}

void Terrain::regenerateTerrain(int newOctaves, float newFrequency, float newAmplitude)
{
    octaves = newOctaves;
    frequency = newFrequency;
    heightScale = newAmplitude;

    generateTerrain();
    setupMesh();
}