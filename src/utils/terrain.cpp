#include "terrain.h"

#include <iostream>

Terrain::Terrain(int width, int depth, float scale, float heightScale, unsigned int seed)
    : width(width), depth(depth), scale(scale), heightScale(heightScale), perlin(seed)
{
    generate(seed);
    setupMesh();
}

Terrain::~Terrain()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

float Terrain::getHeight(float x, float z)
{
    // Sample Perlin noise with multiple octaves for more detail
    float height = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;

    // Add multiple octaves
    for (int i = 0; i < 4; i++)
    {
        height += perlin.noise(x * scale * frequency, 0, z * scale * frequency) * amplitude;
        frequency *= 2.0f;
        amplitude *= 0.5f;
    }

    return height * heightScale;
}

glm::vec3 Terrain::calculateNormal(int x, int z)
{
    // Sample heights around the point
    float heightL = getHeight(x - 1, z);
    float heightR = getHeight(x + 1, z);
    float heightD = getHeight(x, z - 1);
    float heightU = getHeight(x, z + 1);

    // Calculate normal using cross product
    glm::vec3 normal = glm::normalize(glm::vec3(heightL - heightR, 2.0f, heightD - heightU));
    return normal;
}

void Terrain::generate(unsigned int seed)
{
    perlin = PerlinNoise(seed);
    vertices.clear();
    indices.clear();

    // Generate vertices
    for (int z = 0; z < depth; z++)
    {
        for (int x = 0; x < width; x++)
        {
            float xPos = (float)x - width / 2.0f;
            float zPos = (float)z - depth / 2.0f;
            float yPos = getHeight(x, z);

            glm::vec3 normal = calculateNormal(x, z);

            // Position
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);

            // Normal
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);

            // Texture coordinates (optional)
            vertices.push_back((float)x / (float)width);
            vertices.push_back((float)z / (float)depth);
        }
    }

    // Generate indices
    for (int z = 0; z < depth - 1; z++)
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
}

void Terrain::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Terrain::Draw(Shader &shader)
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}