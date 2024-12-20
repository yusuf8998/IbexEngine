#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

#include "Graphics/ShaderObject.h"
#include "Graphics/TextureObject.h"
#include "ResourceManager/ResourceManager.h"
#include "Engine/MeshObject.h"
#include "Engine/Node.h"
#include "Engine/InputHandler.h"
#include <thread>

GLFWwindow *window;

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
    save_thread = std::thread(saveNodeToFile, root, "root.json");
    printf("Saved!\n");
}

int main()
{
    if (!initializeWindow())
    {
        return -1;
    }

    auto mesh = ResourceManager::instance().loadResource<MeshData>("res/cube2.obj");

    MeshObject mesh_obj = MeshObject(mesh.get());
    mesh_obj.generateOpenGLBuffers();

    auto shader_vertex = ResourceManager::instance().loadResource<ShaderData>("res/vertex_mesh.glsl");
    auto shader_frag = ResourceManager::instance().loadResource<ShaderData>("res/fragment_mesh.glsl");

    std::shared_ptr<Node> root = std::make_shared<Node>("root");
    root->addChild(std::make_shared<Node>("dynamic", &mesh_obj, false));

    ShaderObject shader(*shader_vertex.get(), *shader_frag.get());
    shader.use();

    InputHandler inputHandler(window);

    glm::mat4 projection = glm::perspective(glm::radians(60.f), 800.f / 600.f, 0.125f, 10.f);

    InputAxis::Axes["Horizontal"] = InputAxis(GLFW_KEY_D, GLFW_KEY_A, &inputHandler);
    InputAxis::Axes["Vertical"] = InputAxis(GLFW_KEY_W, GLFW_KEY_S, &inputHandler);
    InputAxis::Axes["Z"] = InputAxis(GLFW_KEY_UP, GLFW_KEY_DOWN, &inputHandler);
    InputAxis::Axes["RotationVertical"] = InputAxis(GLFW_KEY_Q, GLFW_KEY_E, &inputHandler);

    auto movementInputVector = InputVector("Horizontal", "Vertical", "Z");
    auto rotationInputVector = InputVector("", "RotationVertical", "");

    // root->children[0]->translate(glm::vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputHandler.update();

        shader.setMat4("projection", projection);

        // Simple update and render cycle
        root->render(&shader);

        root->children[0]->translate(movementInputVector.getValue() / 60.f);
        root->rotate(rotationInputVector.getValue() / 60.f);
        root->updateTransform();

        if (inputHandler.isKeyPressed(GLFW_KEY_P))
        {
            save(root);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    if (save_thread.joinable())
        save_thread.join();
    ResourceManager::instance().clear();
    cleanup();
    return 0;
}