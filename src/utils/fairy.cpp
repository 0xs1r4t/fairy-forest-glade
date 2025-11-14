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
    bodyColor = glm::vec3(0.6f, 0.3f, 0.6f);
    wingColor = glm::vec3(0.8f, 0.6f, 0.9f);
    bodyShininess = 32.0f;
    wingShininess = 64.0f;

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
    // Root: Body
    body.position = glm::vec3(0.0f, 0.0f, -1.0f);
    body.scale = glm::vec3(1.0f);
    body.colour = bodyColor;
    body.shininess = bodyShininess;

    // Left upper wing - child of body
    leftUpperWing.position = glm::vec3(0.15f, 0.15f, 0.0f);
    leftUpperWing.scale = glm::vec3(1.0f);
    leftUpperWing.colour = wingColor;
    leftUpperWing.shininess = wingShininess;
    leftUpperWing.parent = &body;

    // Left lower wing - child of left upper wing
    leftLowerWing.position = glm::vec3(0.0f, -0.15f, 0.0f);
    leftLowerWing.scale = glm::vec3(1.0f);
    leftLowerWing.colour = wingColor;
    leftLowerWing.shininess = wingShininess;
    leftLowerWing.parent = &leftUpperWing;

    // Right upper wing - child of body
    rightUpperWing.position = glm::vec3(-0.15f, 0.15f, 0.0f);
    rightUpperWing.scale = glm::vec3(1.0f);
    rightUpperWing.colour = wingColor;
    rightUpperWing.shininess = wingShininess;
    rightUpperWing.parent = &body;

    // Right lower wing - child of right upper wing
    rightLowerWing.position = glm::vec3(0.0f, -0.15f, 0.0f);
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
    // oscillate from 0 to 1
    float flapPhase = (sin(currentTime * flapSpeed) + 1.0f) * -0.5f;

    float maxFlapAngle = 20.0f;
    float maxLiftAngle = 15.0f;
    float lowerFlapOffset = 0.0f;
    float lowerLiftOffset = 0.75f;

    float wingFlap = flapPhase * maxFlapAngle;
    float wingLift = flapPhase * maxLiftAngle;

    // Upper wings - main flap motion
    leftUpperWing.rotation.y = wingFlap;
    leftLowerWing.rotation.x = wingLift;

    rightUpperWing.rotation.y = -wingFlap;
    rightLowerWing.rotation.x = wingLift;

    // Lower wings - hierarchical motion
    leftLowerWing.rotation.y = wingFlap + lowerFlapOffset;
    leftLowerWing.rotation.x = wingLift * lowerLiftOffset;

    rightLowerWing.rotation.y = -wingFlap - lowerFlapOffset;
    rightLowerWing.rotation.x = wingLift * lowerLiftOffset;
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

void Fairy::RotateAndMoveLeft(float deltaTime)
{
    body.rotation.y += 45.0f * deltaTime; // Rotate left
    float radians = glm::radians(body.rotation.y);
    body.position.x += sin(radians) * flySpeed * deltaTime;
    body.position.z -= cos(radians) * flySpeed * deltaTime;
}

void Fairy::RotateAndMoveRight(float deltaTime)
{
    body.rotation.y -= 45.0f * deltaTime; // Rotate right
    float radians = glm::radians(body.rotation.y);
    body.position.x += sin(radians) * flySpeed * deltaTime;
    body.position.z -= cos(radians) * flySpeed * deltaTime;
}

void Fairy::SetPosition(const glm::vec3 &pos)
{
    body.position = pos;
}

glm::vec3 Fairy::GetPosition() const
{
    return body.position;
}