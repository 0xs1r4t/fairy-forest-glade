#include "shader.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::string Shader::readFile(const char *path)
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

GLuint Shader::compileShader(GLenum type, const char *source)
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

GLuint Shader::createProgramFromFiles(const char *vertPath, const char *fragPath)
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

    glDetachShader(program, vert);
    glDetachShader(program, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    return program;
}

Shader::Shader(const char *vertPath, const char *fragPath)
    : program_(0)
{
    if (vertPath && fragPath)
        program_ = createProgramFromFiles(vertPath, fragPath);
}

Shader::~Shader()
{
    if (program_)
        glDeleteProgram(program_);
}

bool Shader::valid() const { return program_ != 0; }
void Shader::use() const
{
    if (program_)
        glUseProgram(program_);
}
GLuint Shader::id() const { return program_; }

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    if (!program_)
        return;
    GLint loc = glGetUniformLocation(program_, name.c_str());
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, &mat[0][0]);
}