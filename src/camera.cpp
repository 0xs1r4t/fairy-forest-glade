#include "camera.h"

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
    // WASD key inputs to move the camera
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        Position += Orientation * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        Position -= Orientation * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        Position -= glm::normalize(glm::cross(Orientation, Up)) * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        Position += glm::normalize(glm::cross(Orientation, Up)) * speed;
}
