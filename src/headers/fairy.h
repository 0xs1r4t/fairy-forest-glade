#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "model.h"
#include "shader.h"
#include "model_instance.h"

class Fairy
{
public:
    // Constructor - loads all fairy models
    Fairy(const char *bodyPath,
          const char *leftUpperWingPath,
          const char *leftLowerWingPath,
          const char *rightUpperWingPath,
          const char *rightLowerWingPath);

    // Destructor
    ~Fairy();

    // Update animation (call every frame)
    void Update(float currentTime, float deltaTime);

    // Render the fairy and all wings
    void Draw(Shader &shader);

    // Control methods
    void FlyUp(float deltaTime);
    void FlyDown(float deltaTime);
    void RotateAndMoveLeft(float deltaTime);
    void RotateAndMoveRight(float deltaTime);
    void SetPosition(const glm::vec3 &pos);
    glm::vec3 GetPosition() const;

    // Animation parameters (can be tweaked)
    float flapSpeed = 10.0f;
    float hoverSpeed = 2.0f;
    float hoverAmount = 0.05f;
    float flySpeed = 1.0f;

    // Visual parameters
    glm::vec3 bodyColor;
    glm::vec3 wingColor;
    float bodyShininess;
    float wingShininess;

private:
    // Models
    Model *bodyModel;
    Model *leftUpperWingModel;
    Model *leftLowerWingModel;
    Model *rightUpperWingModel;
    Model *rightLowerWingModel;

    // Hierarchical instances
    ModelInstance body;
    ModelInstance leftUpperWing;
    ModelInstance leftLowerWing;
    ModelInstance rightUpperWing;
    ModelInstance rightLowerWing;

    // Animation state
    float hoverOffset;

    // Setup the hierarchy
    void setupHierarchy();

    // Animation logic
    void animateHover(float currentTime, float deltaTime);
    void animateWings(float currentTime);
};