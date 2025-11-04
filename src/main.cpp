#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model.h"
#include "app.h"

// utils
static std::string readFile(const char *path)
{
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in)
    {
        std::cerr << "Failed to open file: " << path << "\n";
        return "";
    }
    std::ostringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

static GLuint compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        GLint len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, ' ');
        glGetShaderInfoLog(shader, len, &len, &log[0]);
        std::cerr << "Shader compile error: " << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint createProgramFromFiles(const char *vertPath, const char *fragPath)
{
    std::string vertSrc = readFile(vertPath);
    std::string fragSrc = readFile(fragPath);
    if (vertSrc.empty() || fragSrc.empty())
    {
        std::cerr << "One or both shader sources are empty\n";
        return 0;
    }

    GLuint vert = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    if (!vert)
        return 0;
    GLuint frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());
    if (!frag)
    {
        glDeleteShader(vert);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        std::string log(len, ' ');
        glGetProgramInfoLog(program, len, &len, &log[0]);
        std::cerr << "Program link error: " << log << "\n";
        glDeleteProgram(program);
        program = 0;
    }

    // shaders can be detached and deleted after linking
    glDetachShader(program, vert);
    glDetachShader(program, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

// glfw window
int main()
{
    const int WIDTH = 1280;
    const int HEIGHT = 960;
    App app(WIDTH, HEIGHT, "Hello World");
    return app.run();
}