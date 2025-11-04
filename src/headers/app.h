#pragma once

/* forward-declare GLFWwindow to avoid pulling in glfw3.h into the header.
   Include <GLFW/glfw3.h> in the .cpp files where needed (so you can control
   that <GL/glew.h> is included before GLFW). */
struct GLFWwindow;
#include <string>
#include "shader.h"
#include "renderer.h"
#include "model.h"

class App
{
public:
    App(int width, int height, const char *title);
    ~App();
    int run();

private:
    bool initGL();
    GLFWwindow *window_;
    int width_, height_;
    std::string title_;
    Shader shader_;
    Renderer renderer_;
    Model *model_; // <--- load model at init time (pointer so we can construct after resolving paths)
};