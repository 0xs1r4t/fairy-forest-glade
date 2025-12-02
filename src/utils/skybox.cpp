#include <iostream>
using namespace std;

#include "skybox.h"
#include "hdri_loader.h"

// Procedural sky constructor (no texture)
Skybox::Skybox() : hdrTexture(0), width(0), height(0), isProcedural(true)
{
    setupSkybox();
    std::cout << "Procedural skybox created" << std::endl;
}

// HDRI sky constructor (with texture)
Skybox::Skybox(const char *hdriPath) : isProcedural(false)
{
    // Load HDRI texture
    hdrTexture = loadHDR(hdriPath, width, height);

    if (hdrTexture == 0)
    {
        cerr << "Failed to load HDRI for skybox!" << endl;
    }
    else
    {
        cout << "HDR texture loaded: " << width << "x" << height << endl;
    }

    // Setup skybox geometry
    setupSkybox();
}

Skybox::~Skybox()
{
    // Clean up OpenGL resources
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    if (hdrTexture != 0)
    {
        glDeleteTextures(1, &hdrTexture);
    }
}

void Skybox::setupSkybox()
{
    float skyboxVertices[] = {
        // positions
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,

        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    // Generate and setup VAO/VBO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

    glBindVertexArray(0);
}

void Skybox::Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection)
{
    // Change depth function so skybox renders behind everything
    glDepthFunc(GL_LEQUAL);

    shader.use();

    // Remove translation from view matrix
    glm::mat4 viewWithoutTranslation = glm::mat4(glm::mat3(view));

    shader.setMat4("view", viewWithoutTranslation);
    shader.setMat4("projection", projection);

    // Only bind texture if using HDRI (not procedural)
    if (!isProcedural && hdrTexture != 0)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, hdrTexture);
        shader.setInt("environmentMap", 0);
    }

    // Draw skybox cube
    glBindVertexArray(skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // Reset depth function
    glDepthFunc(GL_LESS);
}
