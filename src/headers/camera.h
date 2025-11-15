#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.5f;
const float SENSITIVITY = 0.005f;
const float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    // defined for frustum culling
    struct Frustum
    {
        glm::vec4 planes[6]; // left, right, bottom, top, near, far
    };

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    Frustum GetFrustum(float aspect, float fovY, float nearPlane, float farPlane) const
    {
        Frustum frustum;

        const float halfVSide = farPlane * tanf(fovY * 0.5f);
        const float halfHSide = halfVSide * aspect;
        const glm::vec3 frontMultFar = farPlane * Front;

        // Near and far planes
        frustum.planes[4] = glm::vec4(Front, -glm::dot(Front, Position + nearPlane * Front));
        frustum.planes[5] = glm::vec4(-Front, glm::dot(Front, Position + frontMultFar));

        // Left plane
        glm::vec3 leftNormal = glm::normalize(glm::cross(frontMultFar - Right * halfHSide, Up));
        frustum.planes[0] = glm::vec4(leftNormal, -glm::dot(leftNormal, Position));

        // Right plane
        glm::vec3 rightNormal = glm::normalize(glm::cross(Up, frontMultFar + Right * halfHSide));
        frustum.planes[1] = glm::vec4(rightNormal, -glm::dot(rightNormal, Position));

        // Bottom plane
        glm::vec3 bottomNormal = glm::normalize(glm::cross(Right, frontMultFar - Up * halfVSide));
        frustum.planes[2] = glm::vec4(bottomNormal, -glm::dot(bottomNormal, Position));

        // Top plane
        glm::vec3 topNormal = glm::normalize(glm::cross(frontMultFar + Up * halfVSide, Right));
        frustum.planes[3] = glm::vec4(topNormal, -glm::dot(topNormal, Position));

        return frustum;
    }

    // Check if a point is inside the frustum
    bool IsPointInFrustum(const Frustum &frustum, const glm::vec3 &point) const
    {
        for (int i = 0; i < 6; i++)
        {
            if (glm::dot(glm::vec3(frustum.planes[i]), point) + frustum.planes[i].w < 0)
                return false;
        }
        return true;
    }

    // Check if a sphere is inside the frustum (better for foliage culling)
    bool IsSphereInFrustum(const Frustum &frustum, const glm::vec3 &center, float radius) const
    {
        for (int i = 0; i < 6; i++)
        {
            float distance = glm::dot(glm::vec3(frustum.planes[i]), center) + frustum.planes[i].w;
            if (distance < -radius)
                return false;
        }
        return true;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp)); // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};