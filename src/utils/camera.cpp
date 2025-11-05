#define GLM_ENABLE_EXPERIMENTAL
#include "camera.h"

#include <iostream>

/**
 * @brief Construct a new Camera:: Camera object
 *
 * @param width
 * @param height
 * @param position
 *
 * Additional Notes:
 * First Person POV, Perspective View
 * - Camera moves with the player
 * - As if the player is seeing through the camera
 *
 * Third Person POV, Orthographic View
 * - Camera follows the player from a set distance (fixed offset)
 * - Player is always visible on screen
 */

Camera::Camera(int width, int height, glm::vec3 position)
{
    this->width = width;
    this->height = height;
    this->Position = position;

    // Initialize camera values
    this->speed = 0.1f;
    this->sensitivity = 0.1f;
    this->yaw = -90.0f;
    this->pitch = 0.0f;
    this->firstClick = true;
    this->cursorCaptured = false;

    // Calculate initial orientation
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->Orientation = glm::normalize(direction);

    std::cout << "Camera initialized at position: "
              << position.x << ", " << position.y << ", " << position.z << std::endl;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, Shader &shader, const char *uniform)
{
    // perspective projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(FOVdeg), (float)width / (float)height, nearPlane, farPlane);
    shader.setMat4(uniform, projection);
}

// can add orthographic projection later

void Camera::Inputs(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        Position += speed * Orientation;
        std::cout << "W pressed! New position: " << Position.z << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= speed * Orientation;
}