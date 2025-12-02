#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.h"

class Skybox
{
public:
    // Constructors
    Skybox();                     // Procedural sky (no texture)
    Skybox(const char *hdriPath); // HDRI sky (with texture)

    // Destructor - cleans up OpenGL resources
    ~Skybox();

    // Draw the skybox
    void Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);

    // Get the texture ID (for use in other shaders)
    unsigned int GetTextureID() const { return hdrTexture; }

    // Get HDRI dimensions
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    // Check if using procedural sky
    bool IsProcedural() const { return isProcedural; }

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int hdrTexture; // 0 if procedural, texture ID if HDRI
    int width, height;
    bool isProcedural;

    void setupSkybox();
};
