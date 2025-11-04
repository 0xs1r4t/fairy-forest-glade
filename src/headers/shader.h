#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class Shader
{
public:
    explicit Shader(const char *vertPath = nullptr, const char *fragPath = nullptr);
    ~Shader();

    bool valid() const;
    void use() const;
    GLuint id() const;

    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    GLuint program_;
    // helper
    static std::string readFile(const char *path);
    static GLuint compileShader(GLenum type, const char *source);
    static GLuint createProgramFromFiles(const char *vertPath, const char *fragPath);
};
