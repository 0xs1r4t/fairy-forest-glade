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

#define WIDTH 1024
#define HEIGHT 1024

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
    // glEnable(GL_BLEND);
    // glEnable(GL_CULL_FACE);

    // camera + controller setup
    // -------------------------
    Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
    CameraController cameraController(&camera, SCR_WIDTH, SCR_HEIGHT);

    // build and compile our shader program
    // ------------------------------------
    // build and compile shaders
    cout << "Loading shaders..." << endl;
    Shader mainShader("src/shaders/main.vert", "src/shaders/main.frag");
    Shader skyboxShader("src/shaders/skybox.vert", "src/shaders/skybox.frag");
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
    fairy.wingColor = glm::vec3(1.0f, 0.8f, 0.9f);

    // terrain
    // -------
    cout << "Generating terrain..." << endl;
    Terrain terrain(100, 100, 1.0f, 5.0f); // 100x100 grid, 1m spacing, 5m max height
    cout << "Terrain generated!" << endl;

    // draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // camera controls info
    // --------------------
    cout << "\n\n=== CONTROLS ===" << endl;
    cout << "WASD: Move camera" << endl;
    cout << "Mouse: Look around" << endl;
    cout << "ESC: Exit" << endl;
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
        mainShader.setVec3("lightPos", lightPos);
        mainShader.setVec3("lightColor", lightColor);
        mainShader.setVec3("viewPos", camera.Position);

        // Set matrices
        mainShader.setMat4("projection", projection);
        mainShader.setMat4("view", view);

        // ===== UPDATE AND DRAW FAIRY =====
        fairy.Update(currentFrame, g_deltaTime);
        fairy.Draw(mainShader);

        // ===== DRAW TERRAIN =====
        mainShader.use();
        mainShader.setBool("useVertexColor", true); // Use vertex colors for terrain
        glm::mat4 terrainModel = glm::mat4(1.0f);
        terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -5.0f, 0.0f));
        terrain.drawTerrain(mainShader, terrainModel);

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

    // Fairy flight controls
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        fairy.FlyUp(g_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        fairy.FlyDown(g_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        fairy.RotateAndMoveLeft(g_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        fairy.RotateAndMoveRight(g_deltaTime);
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