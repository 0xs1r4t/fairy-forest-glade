#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <algorithm>
using namespace std;

#include "shader.h"
#include "shader_library.h"
#include "camera.h"
#include "camera_controller.h"
#include "lod.h"
#include "skybox.h"
#include "terrain.h"
#include "fairy.h"
#include "firefly.h"
#include "foliage.h"
#include "texture_generator.h"
#include "model.h"
#include "tree_foliage.h"
#include "tree_manager.h"

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

void setupScenePositions(Terrain &terrain, Camera &camera, Fairy &fairy)
{
    // Camera starting position
    float camX = 0.0f, camZ = 15.0f;
    float camHeight = terrain.getHeight(camX, camZ);
    camera.Position = glm::vec3(camX, camHeight + 4.0f, camZ);

    // Fairy starting position
    float fairyX = 0.0f, fairyZ = 8.0f;
    float fairyHeight = terrain.getHeight(fairyX, fairyZ);
    fairy.SetPosition(glm::vec3(fairyX, fairyHeight + 2.5f, fairyZ));

    cout << "Scene positions set - Camera: y=" << camera.Position.y
         << ", Fairy: y=" << fairy.GetPosition().y << endl;
}

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
    glEnable(GL_CULL_FACE);

    // LOD (level-of-detail settings)
    //-------------------------------
    LODConfig grassLOD;
    grassLOD.nearDistance = 15.0f;
    grassLOD.midDistance = 30.0f;
    grassLOD.farDistance = 55.0f;
    grassLOD.nearDensity = 1.0f;
    grassLOD.midDensity = 0.5f;
    grassLOD.farDensity = 0.15f;

    LODConfig flowerLOD;
    flowerLOD.nearDistance = 25.0f;
    flowerLOD.midDistance = 45.0f;
    flowerLOD.farDistance = 80.0f;
    flowerLOD.nearDensity = 1.0f;
    flowerLOD.midDensity = 0.5f;
    flowerLOD.farDensity = 0.2f;

    LODConfig treeLOD;
    treeLOD.nearDistance = 30.0f; // Full detail trees
    treeLOD.midDistance = 60.0f;  // Medium detail
    treeLOD.farDistance = 100.0f; // Far trees

    // build and compile our shader program
    // ------------------------------------
    // build and compile shaders
    cout << "Loading shaders..." << endl;
    Shader mainShader("src/shaders/main.vert", "src/shaders/main.frag");
    // Shader skyboxShader("src/shaders/skybox/skybox.vert", "src/shaders/skybox/skybox.frag");
    Shader skyShader("src/shaders/skybox/procedural_sky.vert", "src/shaders/skybox/procedural_sky.frag");
    Shader terrainShader("src/shaders/terrain/terrain.vert", "src/shaders/terrain/terrain.frag", true);
    Shader grassShader("src/shaders/grass/grass.vert", "src/shaders/grass/grass.frag", true);
    Shader flowerShader("src/shaders/flower/flower.vert", "src/shaders/flower/flower.frag");
    Shader leafShader("src/shaders/tree/leaf.vert", "src/shaders/tree/leaf.frag", true);
    Shader branchShader("src/shaders/tree/branch.vert", "src/shaders/tree/branch.frag", true);
    Shader fireflyShader("src/shaders/firefly/firefly.vert", "src/shaders/firefly/firefly.frag");
    cout << "Shaders loaded successfully!" << endl;

    // Create skybox with HDRI
    cout << "Loading skybox..." << endl;
    Skybox skybox;
    // Skybox skybox("src/assets/textures/satara_night_no_lamps_4k.exr");
    cout << "Skybox loaded!" << endl;

    // terrain setup
    // -------------
    cout << "Generating terrain..." << endl;
    Terrain terrain(100, 100, 1.0f, 12.0f); // 100x100 grid, 1m spacing, 12m max height
    cout << "Terrain generated!" << endl;

    // Calculate terrain area
    float terrainArea = (terrain.width * terrain.scale) * (terrain.height * terrain.scale);
    // For 50x50 @ 1.0 scale = 2500 sq meters
    // For 100x100 @ 1.0 scale = 10000 sq meters

    // Scale foliage counts by area
    float areaRatio = terrainArea / 10000.0f; // Compared to 100x100 terrain

    // camera + controller setup
    // -------------------------
    // start camera at terrain height + offset
    Camera camera(glm::vec3(0.0f, 5.0f, 10.0f)); // Placeholder position
    CameraController cameraController(&camera, SCR_WIDTH, SCR_HEIGHT);

    g_camera = &camera;
    g_cameraController = &cameraController;

    // load fairy models, instances and heirarchy
    // ------------------------------------------
    Fairy fairy("src/assets/models/fairy/fairy_body.obj",
                "src/assets/models/fairy/fairy_upper_left_wing.obj",
                "src/assets/models/fairy/fairy_lower_left_wing.obj",
                "src/assets/models/fairy/fairy_upper_right_wing.obj",
                "src/assets/models/fairy/fairy_lower_right_wing.obj");

    fairy.flapSpeed = 1.0f;
    fairy.wingColor = glm::vec3(0.95f, 0.98f, 1.0f); // Very light cyan-white (glowing)
    fairy.bodyColor = glm::vec3(1.0f, 0.95f, 0.9f);  // Warm glow
    fairy.bodyShininess = 1.0f;
    fairy.wingShininess = 1.0f;

    // basic scene positions setup
    // ---------------------------
    setupScenePositions(terrain, camera, fairy);

    // set fairy's position
    // --------------------
    glm::vec3 fairyStartPos = fairy.GetPosition();

    // create fireflies around fairy
    // -----------------------------
    Firefly fireflies(50, fairy.GetPosition(), 2.5f); // 50 fireflies in 2.5m radius

    // DEBUG: Check if fireflies were created
    auto fireflyPos = fireflies.GetPositions();
    std::cout << "Created " << fireflyPos.size() << " fireflies at fairy position: "
              << fairy.GetPosition().x << ", "
              << fairy.GetPosition().y << ", "
              << fairy.GetPosition().z << std::endl;
    if (fireflyPos.size() > 0)
    {
        std::cout << "First firefly at: " << fireflyPos[0].x << ", "
                  << fireflyPos[0].y << ", "
                  << fireflyPos[0].z << std::endl;
    }

    // ===== CREATE FOLIAGE =====
    cout << "Generating foliage..." << endl;

    // grass
    Foliage grass(&terrain, FoliageType::GRASS,
                  (int)(300000 * areaRatio),
                  0.8f, 0.4f, grassLOD);

    // flowers
    Foliage flowers(&terrain, FoliageType::FLOWER,
                    (int)(5000 * areaRatio),
                    1.0f, 1.0f, flowerLOD);

    // trees
    // load the tree model
    TreeFoliage normalTree("src/assets/models/foliage/trees/NormalTreeBranch.obj");
    normalTree.LoadLeafTextures({"src/assets/textures/Leaves1.PNG",
                                 "src/assets/textures/Leaves2.PNG",
                                 "src/assets/textures/Leaves3.PNG",
                                 "src/assets/textures/Leaves4.PNG"});
    normalTree.GenerateLeafClusters(12, 20); // 12 clusters, 20 leaves each

    TreeFoliage thickTree("src/assets/models/foliage/trees/ThickTreeBranch.obj");
    thickTree.LoadLeafTextures({"src/assets/textures/Leaves1.PNG",
                                "src/assets/textures/Leaves2.PNG",
                                "src/assets/textures/Leaves3.PNG",
                                "src/assets/textures/Leaves4.PNG"});
    thickTree.GenerateLeafClusters(16, 24); // more leaves for thicker tree (denser foliage)

    // tree placements on terrain
    // --------------------------
    TreeManager treeManager(&terrain, &normalTree, &thickTree,
                            (int)(50 * areaRatio), // Scale tree count by area
                            treeLOD,
                            fairyStartPos, // Exclusion center
                            7.0f);         // Exclusion radius

    cout
        << "Foliage generated!" << endl;

    // ===== TEXTURE SETUP (GENERATE PROCEDURAL TEXTURES) =====
    cout << "Generating procedural textures..." << endl;

    // GRASS TEXTURE (512x512)
    unsigned int grassTexture;
    glGenTextures(1, &grassTexture);
    glBindTexture(GL_TEXTURE_2D, grassTexture);

    int gWidth, gHeight, gChannels;
    unsigned char *grassData = stbi_load("src/assets/textures/Grass.png", &gWidth, &gHeight, &gChannels, 0);
    if (grassData)
    {
        GLenum format = (gChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, gWidth, gHeight, 0, format, GL_UNSIGNED_BYTE, grassData);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(grassData);
        cout << "Grass texture loaded!" << endl;
    }

    // FLOWER TEXTURES
    // Load flower textures
    unsigned int flowerTex0, flowerTex1;

    // Flower 1
    glGenTextures(1, &flowerTex0);
    glBindTexture(GL_TEXTURE_2D, flowerTex0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width1, height1, channels1;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data1 = stbi_load("src/assets/textures/flower_1.PNG", &width1, &height1, &channels1, 0);
    if (data1)
    {
        GLenum format = channels1 == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width1, height1, 0, format, GL_UNSIGNED_BYTE, data1);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data1);
        cout << "Flower1 texture loaded! (" << width1 << "x" << height1 << ")" << endl;
    }
    else
    {
        cout << "Failed to load Flower1.png" << endl;
    }

    // Flower 2
    glGenTextures(1, &flowerTex1);
    glBindTexture(GL_TEXTURE_2D, flowerTex1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width2, height2, channels2;
    unsigned char *data2 = stbi_load("src/assets/textures/flower_2.PNG", &width2, &height2, &channels2, 0);
    if (data2)
    {
        GLenum format = channels2 == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width2, height2, 0, format, GL_UNSIGNED_BYTE, data2);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data2);
        cout << "Flower2 texture loaded! (" << width2 << "x" << height2 << ")" << endl;
    }
    else
    {
        cout << "Failed to load Flower2.png" << endl;
    }

    cout << "Textures generated successfully!" << endl;

    cout << "Textures generated successfully!" << endl;

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
        // fps counter
        // -----------
        static float fpsTimer = 0.0f;
        static int fpsCounter = 0;

        fpsTimer += g_deltaTime;
        fpsCounter++;

        if (fpsTimer >= 1.0f)
        {
            cout << "FPS: " << fpsCounter << endl;
            fpsTimer = 0.0f;
            fpsCounter = 0;
        }

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
        glClearColor(0.01f, 0.01f, 0.02f, 1.0f); // Very dark night sky
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup matrices (used by both objects and skybox)
        glm::mat4 projection = glm::perspective(glm::radians(75.0f),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        // ===== ANIMATED MOON =====
        float moonAngle = currentFrame * 0.05f; // Slow rotation
        glm::vec3 moonDirection = glm::normalize(glm::vec3(
            cos(moonAngle) * 0.5f,
            0.6f + 0.2f * sin(moonAngle * 0.3f), // Gentle up/down
            sin(moonAngle) * 0.5f));

        // ===== LIGHTING SETUP =====
        // Moonlight - weak directional light from moon direction
        // glm::vec3 moonDirection = glm::normalize(glm::vec3(0.3f, 0.7f, 0.5f));
        glm::vec3 lightPos = -moonDirection * 20.0f; // Light comes from moon
        glm::vec3 lightColor(0.7f, 0.8f, 1.0f);      // Cool blue moonlight

        // Fairy light - warm point light
        glm::vec3 fairyLightPos = fairy.GetPosition() + glm::vec3(0.0f, 1.5f, 0.0f);
        glm::vec3 fairyLightColor(1.0f, 0.9f, 0.6f); // Warm yellow glow

        // Get firefly positions and colors for lighting
        auto fireflyPositions = fireflies.GetPositions();
        auto fireflyColors = fireflies.GetColors();
        int numFireflyLights = min(8, (int)fireflyPositions.size()); // Max 8 for performance

        // Calculate frustum for foliage culling
        Camera::Frustum frustum = camera.GetFrustum(
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            glm::radians(75.0f),
            0.1f,
            80.0f);

        // ===== UPDATE ANIMATIONS =====
        fairy.Update(currentFrame, g_deltaTime);
        fireflies.Update(g_deltaTime, fairy.GetPosition());

        // ===== DRAW FAIRY (with firefly lighting) =====
        glDisable(GL_CULL_FACE); // for the wings
        mainShader.use();

        // Bind HDRI texture for objects (for reflections/ambient)
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, skybox.GetTextureID());
        mainShader.setInt("environmentMap", 0);

        // Set matrices
        mainShader.setMat4("projection", projection);
        mainShader.setMat4("view", view);
        mainShader.setVec3("viewPos", camera.Position);

        // Set lighting uniforms
        mainShader.setVec3("lightPos", lightPos);
        mainShader.setVec3("lightColor", lightColor);
        mainShader.setVec3("fairyLightPos", fairyLightPos);
        mainShader.setVec3("fairyLightColor", fairyLightColor);

        // Pass firefly lights (up to 8 closest ones)
        mainShader.setInt("numFireflies", numFireflyLights);
        for (int i = 0; i < numFireflyLights; i++)
        {
            string posName = "fireflyPositions[" + to_string(i) + "]";
            string colorName = "fireflyColors[" + to_string(i) + "]";
            mainShader.setVec3(posName, fireflyPositions[i]);
            mainShader.setVec3(colorName, fireflyColors[i]);
        }

        fairy.Draw(mainShader);
        glEnable(GL_CULL_FACE); // re-enable culling after fairy wings are rendered

        // ===== DRAW TERRAIN =====
        terrainShader.use();
        terrainShader.setVec3("lightPos", lightPos);
        terrainShader.setVec3("lightColor", lightColor);
        terrainShader.setVec3("viewPos", camera.Position);
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view", view);

        glm::mat4 terrainModel = glm::mat4(1.0f);
        terrain.drawTerrain(terrainShader, terrainModel);

        // disable backface culling for foliage only
        // glDisable(GL_CULL_FACE);

        // ===== DRAW GRASS =====
        grassShader.use();
        grassShader.setFloat("time", (float)glfwGetTime());
        grassShader.setMat4("view", view);
        grassShader.setMat4("projection", projection);
        grassShader.setVec3("cameraPos", camera.Position);
        grassShader.setVec3("lightDir", glm::vec3(0.3f, -0.7f, 0.5f));
        grassShader.setVec3("ambientColor", glm::vec3(0.15f, 0.2f, 0.25f));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        grassShader.setInt("grassTexture", 0);

        grass.Draw(grassShader, view, projection, frustum, camera);

        // ===== DRAW FLOWERS =====
        if (flowerTex0 != 0 && flowerTex1 != 0)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            flowerShader.use();
            flowerShader.setFloat("time", (float)glfwGetTime()); // ADD THIS
            flowerShader.setMat4("view", view);
            flowerShader.setMat4("projection", projection);
            flowerShader.setVec3("fairyPos", fairy.GetPosition());
            flowerShader.setFloat("fairyRadius", 3.0f);
            flowerShader.setVec3("viewPos", camera.Position);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, flowerTex0);
            flowerShader.setInt("flowerTexture0", 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, flowerTex1);
            flowerShader.setInt("flowerTexture1", 1);

            flowers.Draw(flowerShader, view, projection, frustum, camera);

            glDisable(GL_BLEND);
        }

        else
        {
            // Only show once
            static bool warned = false;
            if (!warned)
            {
                cout << "Skipping flower rendering - textures not loaded" << endl;
                warned = true;
            }
        }

        // ===== DRAW TREES =====
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        leafShader.use();
        leafShader.setFloat("time", (float)glfwGetTime());
        leafShader.setVec3("lightDir", glm::vec3(0.3f, -0.7f, 0.5f));
        leafShader.setVec3("lightColor", lightColor);
        leafShader.setVec3("ambientColor", glm::vec3(0.15f, 0.2f, 0.25f));

        branchShader.use();
        branchShader.setFloat("time", (float)glfwGetTime());

        // multiple trees at different positions on the terrain
        treeManager.Draw(leafShader, branchShader, view, projection, frustum, camera);

        glDisable(GL_BLEND);

        // ===== DRAW FIREFLIES =====
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending
        glDepthMask(GL_FALSE);

        // DEBUG - check positions before drawing
        static int frameDebug = 0;
        if (frameDebug++ % 120 == 0)
        { // Every 2 seconds
            auto pos = fireflies.GetPositions();
            std::cout << "Firefly count: " << pos.size() << std::endl;
            if (pos.size() > 0)
            {
                std::cout << "  First firefly: " << pos[0].x << ", " << pos[0].y << ", " << pos[0].z << std::endl;
                std::cout << "  Camera pos: " << camera.Position.x << ", " << camera.Position.y << ", " << camera.Position.z << std::endl;
                std::cout << "  Distance: " << glm::distance(pos[0], camera.Position) << std::endl;
            }
        }

        fireflyShader.use();
        fireflyShader.setFloat("time", currentFrame);
        fireflyShader.setMat4("view", view);
        fireflyShader.setMat4("projection", projection);

        fireflies.Draw(fireflyShader, view, projection);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        // ===== DRAW SKYBOX (LAST) =====
        // skybox.Draw(skyboxShader, view, projection);
        skyShader.use();

        // Pass time for twinkling stars
        skyShader.setFloat("time", currentFrame);

        // Pass moon direction (same as your main light!)
        skyShader.setVec3("moonDir", moonDirection);

        // Set matrices
        glm::mat4 skyView = glm::mat4(glm::mat3(view)); // Remove translation
        skyShader.setMat4("view", skyView);
        skyShader.setMat4("projection", projection);

        // Draw skybox
        skybox.Draw(skyShader, skyView, projection);

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