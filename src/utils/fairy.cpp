#include "fairy.h"

#include <iostream>
using namespace std;

Fairy::Fairy(const char *bodyPath,
             const char *leftUpperWingPath,
             const char *leftLowerWingPath,
             const char *rightUpperWingPath,
             const char *rightLowerWingPath)
    : hoverOffset(0.0f)
{
    cout << "Loading fairy models..." << endl;

    // Load all models
    bodyModel = new Model(bodyPath);
    leftUpperWingModel = new Model(leftUpperWingPath);
    leftLowerWingModel = new Model(leftLowerWingPath);
    rightUpperWingModel = new Model(rightUpperWingPath);
    rightLowerWingModel = new Model(rightLowerWingPath);

    cout << "Fairy models loaded successfully!" << endl;

    // Set default colors and properties
    bodyColor = glm::vec3(0.95f, 0.7f, 0.95f); // Light pink/lavender
    wingColor = glm::vec3(0.9f, 0.95f, 1.0f);  // Light blue-white
    bodyShininess = 1.0f;                      // Very minimal shine
    wingShininess = 1.0f;                      // Very minimal shine

    // Setup the hierarchical structure
    setupHierarchy();
}

Fairy::~Fairy()
{
    delete bodyModel;
    delete leftUpperWingModel;
    delete leftLowerWingModel;
    delete rightUpperWingModel;
    delete rightLowerWingModel;
}

void Fairy::setupHierarchy()
{

    float distance = 0.2f;

    // Root: Body
    body.position = glm::vec3(0.0f, 5.0f, 5.0f);
    body.scale = glm::vec3(1.0f);
    body.colour = bodyColor;
    body.shininess = bodyShininess;

    // Left upper wing - child of body
    leftUpperWing.position = glm::vec3(0.15f - distance, 0.15f, 0.0f);
    leftUpperWing.scale = glm::vec3(1.0f);
    leftUpperWing.colour = wingColor;
    leftUpperWing.shininess = wingShininess;
    leftUpperWing.parent = &body;

    // Left lower wing - child of left upper wing
    leftLowerWing.position = glm::vec3(0.0f - distance, -0.15f, 0.0f);
    leftLowerWing.scale = glm::vec3(1.0f);
    leftLowerWing.colour = wingColor;
    leftLowerWing.shininess = wingShininess;
    leftLowerWing.parent = &leftUpperWing;

    // Right upper wing - child of body
    rightUpperWing.position = glm::vec3(-0.15f + distance, 0.15f, 0.0f);
    rightUpperWing.scale = glm::vec3(1.0f);
    rightUpperWing.colour = wingColor;
    rightUpperWing.shininess = wingShininess;
    rightUpperWing.parent = &body;

    // Right lower wing - child of right upper wing
    rightLowerWing.position = glm::vec3(0.0f + distance, -0.15f, 0.0f);
    rightLowerWing.scale = glm::vec3(1.0f);
    rightLowerWing.colour = wingColor;
    rightLowerWing.shininess = wingShininess;
    rightLowerWing.parent = &rightUpperWing;
}

void Fairy::Update(float currentTime, float deltaTime)
{
    animateHover(currentTime, deltaTime);
    animateWings(currentTime);
}

void Fairy::animateHover(float currentTime, float deltaTime)
{
    hoverOffset = sin(currentTime * hoverSpeed) * hoverAmount;
    body.position.y += hoverOffset * deltaTime * 10.0f;
}

void Fairy::animateWings(float currentTime)
{
    // Two different phases for upper and lower wings
    float upperPhase = (sin(currentTime * flapSpeed) + 1.0f) * 0.5f;
    float lowerPhase = (sin(currentTime * flapSpeed + 0.3f) + 1.0f) * 0.5f; // Slight delay

    float maxFlapAngle = 20.0f;
    float maxLiftAngle = 15.0f;

    // Upper wings
    float upperFlap = upperPhase * maxFlapAngle;
    leftUpperWing.rotation.y = upperFlap;
    rightUpperWing.rotation.y = -upperFlap;

    // Lower wings - reduced motion, slight phase offset
    float lowerFlap = lowerPhase * maxFlapAngle * 0.5f; // Half the range
    float lowerLift = lowerPhase * maxLiftAngle * 0.4f; // Less lift

    leftLowerWing.rotation.y = lowerFlap;
    leftLowerWing.rotation.x = lowerLift;

    rightLowerWing.rotation.y = -lowerFlap;
    rightLowerWing.rotation.x = lowerLift;
}

void Fairy::Draw(Shader &shader)
{
    shader.setBool("useVertexColor", false);

    // Draw body (root)
    glm::mat4 globalBody = body.GetModelMatrix();
    shader.setVec3("materialColor", body.colour);
    shader.setFloat("materialShininess", body.shininess);
    shader.setMat4("model", globalBody);
    bodyModel->Draw(shader);

    // Draw left wing branch
    glm::mat4 globalLeftUpper = leftUpperWing.GetModelMatrix();
    shader.setVec3("materialColor", leftUpperWing.colour);
    shader.setFloat("materialShininess", leftUpperWing.shininess);
    shader.setMat4("model", globalLeftUpper);
    leftUpperWingModel->Draw(shader);

    glm::mat4 globalLeftLower = leftLowerWing.GetModelMatrix();
    shader.setMat4("model", globalLeftLower);
    leftLowerWingModel->Draw(shader);

    // Draw right wing branch
    glm::mat4 globalRightUpper = rightUpperWing.GetModelMatrix();
    shader.setVec3("materialColor", rightUpperWing.colour);
    shader.setFloat("materialShininess", rightUpperWing.shininess);
    shader.setMat4("model", globalRightUpper);
    rightUpperWingModel->Draw(shader);

    glm::mat4 globalRightLower = rightLowerWing.GetModelMatrix();
    shader.setMat4("model", globalRightLower);
    rightLowerWingModel->Draw(shader);
}

void Fairy::FlyUp(float deltaTime)
{
    body.position.y += flySpeed * deltaTime;
}

void Fairy::FlyDown(float deltaTime)
{
    body.position.y -= flySpeed * deltaTime;
}

// Move forward in the direction the fairy is facing
void Fairy::MoveForward(float deltaTime)
{
    float radians = glm::radians(body.rotation.y);
    body.position.x += sin(radians) * flySpeed * deltaTime;
    body.position.z -= cos(radians) * flySpeed * deltaTime;
}

// Move backward (opposite of facing direction)
void Fairy::MoveBackward(float deltaTime)
{
    float radians = glm::radians(body.rotation.y);
    body.position.x -= sin(radians) * flySpeed * deltaTime;
    body.position.z += cos(radians) * flySpeed * deltaTime;
}

// Strafe left (perpendicular to facing direction)
void Fairy::MoveLeft(float deltaTime)
{
    float radians = glm::radians(body.rotation.y - 90.0f);
    body.position.x += sin(radians) * flySpeed * deltaTime;
    body.position.z -= cos(radians) * flySpeed * deltaTime;
}

// Strafe right (perpendicular to facing direction)
void Fairy::MoveRight(float deltaTime)
{
    float radians = glm::radians(body.rotation.y + 90.0f);
    body.position.x += sin(radians) * flySpeed * deltaTime;
    body.position.z -= cos(radians) * flySpeed * deltaTime;
}

// Rotate left (counterclockwise) without moving
void Fairy::RotateLeft(float deltaTime)
{
    body.rotation.y += 90.0f * deltaTime; // 90 degrees per second
}

// Rotate right (clockwise) without moving
void Fairy::RotateRight(float deltaTime)
{
    body.rotation.y -= 90.0f * deltaTime; // 90 degrees per second
}

void Fairy::SetPosition(const glm::vec3 &pos)
{
    body.position = pos;
}

glm::vec3 Fairy::GetPosition() const
{
    return body.position;
}