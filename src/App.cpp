#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

#include "Graphics/ShaderObject.h"
#include "Graphics/TextureObject.h"
#include "ResourceManager/ResourceManager.h"
#include "Graphics/MeshObject.h"
#include "Engine/SceneGraph.h"
#include "Engine/InputHandler.h"
#include "Engine/Camera.h"
#include <thread>

GLFWwindow *window;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    mainCamera.processMouseScroll((float)yoffset);
}

bool initializeWindow()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    window = glfwCreateWindow(800, 600, "Ibex Engine", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    return true;
}

void cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

std::thread save_thread;

void save(const std::shared_ptr<Node> &root)
{
    if (save_thread.joinable())
        save_thread.join();
    save_thread = std::thread(saveSceneGraph, "root.json", root);
    printf("Saved!\n");
}

// Delta time
float deltaTime = 0.f;
float lastFrame = 0.f;
float currentFrame = 0.f;

double deltaMouseX = 0.f, deltaMouseY = 0.f;

unsigned long mouseState = GLFW_CURSOR_NORMAL;

int main()
{
    if (!initializeWindow())
    {
        return -1;
    }

    auto shader_vertex = ResourceManager::instance().loadResource<ShaderData>("res/vertex_mesh.glsl");
    auto shader_frag = ResourceManager::instance().loadResource<ShaderData>("res/fragment_mesh.glsl");

    std::shared_ptr<Node> root;
    loadSceneGraph("root.json", root);

    ShaderObject shader(*shader_vertex.get(), *shader_frag.get());
    shader.use();

    InputHandler inputHandler(window);

    glm::mat4 projection, view;

    InputAxis::Axes["Horizontal"] = InputAxis(GLFW_KEY_D, GLFW_KEY_A, &inputHandler);
    InputAxis::Axes["Vertical"] = InputAxis(GLFW_KEY_S, GLFW_KEY_W, &inputHandler);
    InputAxis::Axes["Z"] = InputAxis(GLFW_KEY_E, GLFW_KEY_Q, &inputHandler);
    InputAxis::Axes["RotationVertical"] = InputAxis(GLFW_KEY_UP, GLFW_KEY_DOWN, &inputHandler);
    InputAxis::Axes["RotationHorizontal"] = InputAxis(GLFW_KEY_RIGHT, GLFW_KEY_LEFT, &inputHandler);

    auto movementInputVector = InputVector("Horizontal", "Z", "Vertical");
    auto rotationInputVector = InputVector("RotationHorizontal", "RotationVertical", "");

    glm::vec4 transformedInput;

    while (!glfwWindowShouldClose(window))
    {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputHandler.update();

        inputHandler.getMouseDelta(deltaMouseX, deltaMouseY);
        mainCamera.processMouseMovement(deltaMouseX, deltaMouseY);

        if (inputHandler.isKeyPressed(GLFW_KEY_P))
        {
            save(root);
        }

        if (inputHandler.isKeyPressed(GLFW_KEY_N))
        {
            castNode<Renderable>(root->children[0])->visible = !castNode<Renderable>(root->children[0])->visible;
        }

        if (inputHandler.isKeyPressed(GLFW_KEY_ESCAPE))
        {
            mouseState = mouseState == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
            glfwSetInputMode(window, GLFW_CURSOR, mouseState);
        }

        transformedInput = glm::vec4(movementInputVector.getValue(), 1.f);
        transformedInput = mainCamera.getRotationMatrix() * transformedInput;
        mainCamera.position += glm::vec3(transformedInput) * deltaTime;

        // Setup view and projection matrices
        view = mainCamera.getViewMatrix();
        projection = glm::perspective(glm::radians(mainCamera.zoom), 800.f / 600.f, 0.1f, 100.f);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // Simple update and render cycle
        updateSceneGraph(root);
        renderSceneGraph(root, &shader);

        castNode<Transformable>(root)->getTransform().rotate(rotationInputVector.getValue() * deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    if (save_thread.joinable())
        save_thread.join();
    ResourceManager::instance().clear();
    cleanup();
    return 0;
}