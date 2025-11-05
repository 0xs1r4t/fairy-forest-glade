#define GLM_ENABLE_EXPERIMENTAL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>

#include "camera.h"
#include "app.h"

App::App(int width, int height, const char *title)
    : window_(nullptr),
      width_(width),
      height_(height),
      title_(title),
      shader_(nullptr, nullptr),
      renderer_(),
      model_(nullptr),
      camera_(width, height, glm::vec3(0.0f, 0.0f, 3.0f)) // Initialize camera with starting position
{
}

App::~App()
{
    if (model_)
        delete model_;
    if (shader_.valid())
        ; // shader destructor cleans up
    if (window_)
    {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

bool App::initGL()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!window_)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }

    // resolve resource paths relative to current working directory and verify
    namespace fs = std::filesystem;
    fs::path base = fs::current_path(); // prints helpful info if needed
    fs::path vertPath = base / "src" / "shaders" / "model.vert";
    fs::path fragPath = base / "src" / "shaders" / "model.frag";
    fs::path modelPath = base / "src" / "models" / "monkey.obj";

    std::cout << "Using shader: " << vertPath << " and " << fragPath << std::endl;
    std::cout << "Using model: " << modelPath << std::endl;

    if (!fs::exists(vertPath) || !fs::exists(fragPath))
    {
        std::cerr << "Shader files not found. Make sure paths are correct relative to working dir: " << base << std::endl;
        return false;
    }

    if (!fs::exists(modelPath))
    {
        std::cerr << "Model file not found. Make sure paths are correct relative to working dir: " << base << std::endl;
        return false;
    }

    // create shader program
    shader_ = Shader(vertPath.string().c_str(), fragPath.string().c_str());
    if (!shader_.valid())
    {
        std::cerr << "Failed to create shader program\n";
        return false;
    }

    // load model after verifying path
    model_ = new Model(modelPath.string().c_str());

    renderer_.init();

    return true;
}

int App::run()
{
    if (!initGL())
        return -1;

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set initial cursor mode to disabled for camera control
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    camera_.cursorCaptured = true;
    std::cout << "Cursor captured set to: " << camera_.cursorCaptured << std::endl;

    // Set a reasonable starting position and speed
    camera_.Position = glm::vec3(0.0f, 2.0f, 5.0f);
    camera_.speed = 0.1f;       // Increase speed for better control
    camera_.sensitivity = 0.1f; // Reduce sensitivity for smoother camera

    while (!glfwWindowShouldClose(window_))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_.use();

        // Handle camera movement
        camera_.Inputs(window_);

        // Update camera matrices
        camera_.Matrix(45.0f, 0.1f, 500.0f, shader_, "projection");

        glm::mat4 view = glm::lookAt(
            camera_.Position,
            camera_.Position + camera_.Orientation,
            camera_.Up);

        std::cout << "View matrix at: "
                  << camera_.Position.x << ", "
                  << camera_.Position.y << ", "
                  << camera_.Position.z
                  << " looking at: "
                  << (camera_.Position + camera_.Orientation).x << ", "
                  << (camera_.Position + camera_.Orientation).y << ", "
                  << (camera_.Position + camera_.Orientation).z << std::endl;

        shader_.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, -10.0f)); // Move model to Y=2 (same height as camera) and further away
        shader_.setMat4("model", model);

        // simple model/view/proj
        // glm::mat4 model = glm::mat4(1.0f);
        // glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
        //                              glm::vec3(0.0f, 0.0f, 0.0f),
        //                              glm::vec3(0.0f, 1.0f, 0.0f));
        // glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        //                                         (float)width_ / (float)height_, 0.1f, 100.0f);

        // draw loaded model (check pointer)
        if (model_)
            renderer_.drawModel(*model_);

        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    return 0;
}