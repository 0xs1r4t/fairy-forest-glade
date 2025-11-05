#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "shader.h"

class Camera
{
public:
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

    int width;
    int height;

    // Modify these values for better control
    float speed = 0.1f;
    float sensitivity = 0.1f;

    // Add middle mouse pan support
    bool isPanning = false;
    bool isOrbiting = false;
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    float orbitDistance = 5.0f;

    // Add these new members
    bool firstClick = true;
    bool cursorCaptured = false;
    float yaw = -90.0f;
    float pitch = 0.0f;
    double lastX = 0.0f;
    double lastY = 0.0f;

    Camera(int width, int height, glm::vec3 position);

    void Matrix(float FOVdeg, float nearPlane, float farPlane, Shader &shader, const char *uniform);
    void Inputs(GLFWwindow *window);
};