#include "camera_controller.h"

CameraController::CameraController(Camera *cam, float screenWidth, float screenHeight)
    : camera(cam),
      lastX(screenWidth / 2.0f),
      lastY(screenHeight / 2.0f),
      firstMouse(true),
      mouseSensitivity(1.0f)
{
}

void CameraController::ProcessKeyboard(GLFWwindow *window, float deltaTime)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaTime);
}

void CameraController::ProcessMouseMovement(double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera->ProcessMouseMovement(xoffset, yoffset);
}

void CameraController::ProcessMouseScroll(double yoffset)
{
    camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

void CameraController::ProcessFairyMovement(GLFWwindow *window, float deltaTime, Fairy &fairy)
{
    // Horizontal movement (arrow keys)
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        fairy.MoveForward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        fairy.MoveBackward(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        fairy.MoveLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        fairy.MoveRight(deltaTime);

    // Vertical movement (I/K - up/down)
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        fairy.FlyUp(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        fairy.FlyDown(deltaTime);

    // Rotation only (J/L - left/right)
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        fairy.RotateLeft(deltaTime);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        fairy.RotateRight(deltaTime);
}