#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "camera.h"

class CameraController
{
public:
    // Constructor
    CameraController(Camera *cam, float screenWidth, float screenHeight);

    // Process keyboard input
    void ProcessKeyboard(GLFWwindow *window, float deltaTime);

    // Process mouse movement
    void ProcessMouseMovement(double xpos, double ypos);

    // Process mouse scroll
    void ProcessMouseScroll(double yoffset);

    // Get the camera
    Camera *GetCamera() { return camera; }

    // Mouse sensitivity (can be adjusted)
    float mouseSensitivity;

private:
    Camera *camera;

    // Mouse state
    float lastX;
    float lastY;
    float lastZ;
    bool firstMouse;
};