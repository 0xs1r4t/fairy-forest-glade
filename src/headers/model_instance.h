#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class ModelInstance
{
public:
    glm::vec3 position;
    glm::vec3 rotation; // Euler angles in degrees
    glm::vec3 scale;
    glm::vec3 colour;
    float shininess;

    ModelInstance *parent; // For hierarchical transforms

    ModelInstance()
        : position(0.0f), rotation(0.0f), scale(1.0f),
          colour(1.0f), shininess(32.0f), parent(nullptr) {}

    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 model = glm::mat4(1.0f);

        // Apply transformations in order: Scale -> Rotate -> Translate
        model = glm::translate(model, position);
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, scale);

        // Apply parent transform if exists
        if (parent != nullptr)
        {
            model = parent->GetModelMatrix() * model;
        }

        return model;
    }
};