#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
using namespace std;

#include "shader.h"
#include "camera.h"
#include "camera_controller.h"
#include "skybox.h"
#include "terrain.h"
#include "fairy.h"
#include "grass.h"
#include "flowers.h"
#include "model.h"

#define WIDTH 1920
#define HEIGHT 1200

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window, CameraController &cameraController, Fairy &fairy);

// screen settings
const unsigned int SCR_WIDTH = WIDTH;
const unsigned int SCR_HEIGHT = HEIGHT;

// camera
Camera *g_camera = nullptr;
CameraController *g_cameraController = nullptr;

// timing
float g_deltaTime = 0.0f;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fairy Forest Glade", NULL, NULL);
    if (window == NULL)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glew: initialise and load all OpenGL function pointers
    // ---------------------------------------
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }

    // Check for OpenGL errors
    cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    // glEnable(GL_CULL_FACE);

    // camera + controller setup
    // -------------------------
    // Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    Camera camera(glm::vec3(0.0f, 5.0f, 10.0f));
    CameraController cameraController(&camera, SCR_WIDTH, SCR_HEIGHT);

    g_camera = &camera;
    g_cameraController = &cameraController;

    // build and compile our shader program
    // ------------------------------------
    // build and compile shaders
    cout << "Loading shaders..." << endl;
    Shader mainShader("src/shaders/main.vert", "src/shaders/main.frag");
    Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");
    Shader terrainShader("src/shaders/terrain.vert", "src/shaders/terrain.frag");
    Shader grassShader("src/shaders/grass.vert", "src/shaders/grass.frag");
    Shader flowerShader("src/shaders/flower.vert", "src/shaders/flower.frag");
    cout << "Shaders loaded successfully!" << endl;

    // Create skybox with HDRI
    cout << "Loading skybox..." << endl;
    Skybox skybox("src/assets/textures/satara_night_no_lamps_4k.exr");
    cout << "Skybox loaded!" << endl;

    // load fairy models, instances and heirarchy
    // ------------------------------------------
    Fairy fairy("src/assets/models/fairy/fairy_body.obj",
                "src/assets/models/fairy/fairy_upper_left_wing.obj",
                "src/assets/models/fairy/fairy_lower_left_wing.obj",
                "src/assets/models/fairy/fairy_upper_right_wing.obj",
                "src/assets/models/fairy/fairy_lower_right_wing.obj");

    fairy.flapSpeed = 1.0f;
    fairy.wingColor = glm::vec3(0.95f, 0.98f, 1.0f); // Very light cyan-white
    fairy.bodyColor = glm::vec3(0.95f, 0.8f, 0.95f); // Light lavender
    fairy.bodyShininess = 1.0f;
    fairy.wingShininess = 1.0f;

    // terrain
    // -------
    cout << "Generating terrain..." << endl;
    Terrain terrain(100, 100, 1.0f, 5.0f); // 100x100 grid, 1m spacing, 5m max height
    cout << "Terrain generated!" << endl;

    // grass
    cout << "Generating grass..." << endl;
    Model grassModel("src/assets/models/foliage/grass.obj");
    Grass grass(&terrain, &grassModel, 10000); // number of instances of grass on the terrain

    // flowers
    cout << "Generating flowers..." << endl;
    Model flowerModel("src/assets/models/foliage/flowers.obj");
    Flowers flowers(&terrain, &flowerModel, 2000); // number of instances of flowers on the terrain

    cout << "=== FLOWER DEBUG ===" << endl;
    cout << "Flower positions generated: " << flowers.positions.size() << endl;
    cout << "Flower model meshes: " << flowerModel.meshes.size() << endl;
    if (!flowers.positions.empty())
    {
        cout << "First flower at: (" << flowers.positions[0].x << ", "
             << flowers.positions[0].y << ", " << flowers.positions[0].z << ")" << endl;
    }
    cout << "Camera starts at: (" << camera.Position.x << ", "
         << camera.Position.y << ", " << camera.Position.z << ")" << endl;
    cout << "===================" << endl;

    // ===== TEXTURE SETUP =====

    // GRASS TEXTURE setup
    unsigned int grassTex;
    glGenTextures(1, &grassTex);
    int w, h, channels;
    unsigned char *data = stbi_load("src/assets/textures/grass.png", &w, &h, &channels, 4);
    glBindTexture(GL_TEXTURE_2D, grassTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // FLOWER TEXTURE setup
    unsigned int flowerTex;
    glGenTextures(1, &flowerTex);
    data = stbi_load("src/assets/textures/flower.png", &w, &h, &channels, 4);
    glBindTexture(GL_TEXTURE_2D, flowerTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // camera controls info
    // --------------------
    cout << "\n\n=== CONTROLS ===\n"
         << endl;
    cout << "Camera Controls (Left Hand):" << endl;
    cout << "  WASD: Move camera" << endl;
    cout << "  Mouse: Look around" << endl;
    cout << "  Scroll: Zoom in/out\n"
         << endl;

    cout << "Fairy Controls (Right Hand):" << endl;
    cout << "  Arrow Keys: Move fairy (forward/back/left/right)" << endl;
    cout << "  I/K: Fly up/down" << endl;
    cout << "  J/L: Rotate left/right\n"
         << endl;

    cout << "ESC: Exit" << endl;
    cout << "===================\n"
         << endl;

    cout << "\nStarting render loop..." << endl;
    // timing
    float lastFrame = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        g_deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, cameraController, fairy);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // bind textures on corresponding texture units

        // Setup matrices (used by both objects and skybox)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ===== DRAW OBJECTS =====
        mainShader.use();

        // Bind HDRI texture for objects (for reflections/ambient)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skybox.GetTextureID());
        mainShader.setInt("environmentMap", 0);

        // Lighting uniforms
        glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
        glm::vec3 fairyLightPos = fairy.GetPosition() + glm::vec3(0.0f, 1.5f, 0.0f);
        glm::vec3 fairyLightColor(1.0f, 0.9f, 0.6f); // Warm glow

        mainShader.setVec3("lightPos", lightPos);
        mainShader.setVec3("lightColor", lightColor);
        mainShader.setVec3("fairyLightPos", fairyLightPos);
        mainShader.setVec3("fairyLightColor", fairyLightColor);
        mainShader.setVec3("viewPos", camera.Position);

        // Set matrices
        mainShader.setMat4("projection", projection);
        mainShader.setMat4("view", view);

        // Calculate frustum and pass it to all foliage for culling
        Camera::Frustum frustum = camera.GetFrustum(
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            glm::radians(camera.Zoom),
            0.1f,
            100.0f);

        // ===== UPDATE AND DRAW FAIRY =====
        fairy.Update(currentFrame, g_deltaTime);
        fairy.Draw(mainShader);

        // ===== DRAW TERRAIN =====
        terrainShader.use();
        terrainShader.setVec3("lightPos", lightPos);
        terrainShader.setVec3("lightColor", lightColor);
        terrainShader.setVec3("viewPos", camera.Position);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);

        glm::mat4 terrainModel = glm::mat4(1.0f);
        // terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -5.0f, 0.0f));
        terrain.drawTerrain(terrainShader, terrainModel);

        // ===== DRAW GRASS =====
        grassShader.use();
        grassShader.setMat4("view", view);
        grassShader.setMat4("projection", projection);
        grassShader.setVec3("lightPos", lightPos);
        grassShader.setVec3("viewPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTex);
        grassShader.setInt("grassTexture", 0);
        grass.Draw(grassShader, view, projection, frustum, camera);

        // ===== DRAW FLOWERS =====
        flowerShader.use();
        flowerShader.setMat4("view", view);
        flowerShader.setMat4("projection", projection);
        flowerShader.setVec3("lightPos", lightPos);
        flowerShader.setVec3("viewPos", camera.Position);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, flowerTex);
        flowerShader.setInt("flowerTexture", 0);
        flowers.Draw(flowerShader, view, projection, frustum, camera);

        // ===== DRAW SKYBOX (LAST) =====
        skybox.Draw(skyboxShader, view, projection);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFWresources.
    //---------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window, CameraController &cameraController, Fairy &fairy)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera controls (delegated to controller)
    cameraController.ProcessKeyboard(window, g_deltaTime);

    // Fairy controls
    cameraController.ProcessFairyMovement(window, g_deltaTime, fairy);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (g_cameraController)
        g_cameraController->ProcessMouseMovement(xpos, ypos);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (g_cameraController)
        g_cameraController->ProcessMouseScroll(yoffset);
}