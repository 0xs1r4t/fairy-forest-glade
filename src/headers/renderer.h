#pragma once

#include <GL/glew.h>
#include "model.h"
#include "shader.h"

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void init();
    void drawTriangle(const Shader &shader) const;
    void drawModel(const Model &model) const;

private:
    GLuint vao_, vbo_;
    bool initialized_;
};
