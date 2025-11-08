#include <GL/glew.h>
#include <glm/glm.hpp>

#include "shader.h"

class Skybox
{
public:
    // Constructor - loads HDRI and sets up skybox
    Skybox(const char *hdriPath);

    // Destructor - cleans up OpenGL resources
    ~Skybox();

    // Draw the skybox
    void Draw(Shader &shader, const glm::mat4 &view, const glm::mat4 &projection);

    // Get the HDRI texture ID (for use in other shaders)
    unsigned int GetTextureID() const { return hdrTexture; }

    // Get HDRI dimensions
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int hdrTexture;
    int width, height;

    void setupSkybox();
};
