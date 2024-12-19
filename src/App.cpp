#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

#include "Graphics/ShaderObject.h"
#include "Graphics/TextureObject.h"
#include "ResourceManager/ResourceManager.h"
#include "Engine/Mesh.h"
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

    window = glfwCreateWindow(800, 600, "Scene Graph Example", nullptr, nullptr);
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
    save_thread = std::thread(saveNodeToFile, root, "root.json");
}

int main()
{
    if (!initializeWindow())
    {
        return -1;
    }

    // Create mesh (a simple triangle for example)
    std::vector<Vertex> vertices = {
        {{+0.0f, +0.5f, +0.0f}, {1.0f, 1.0f, 1.0f}, {0.5f, 1.0f}},
        {{+0.5f, -0.5f, +0.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, +0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}};
    std::vector<unsigned int> indices = {0, 1, 2};

    std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>("tri", vertices, indices);

    auto texture1 = ResourceManager::instance().loadResource<TextureData>("res/box.png");

    auto shader_vertex = ResourceManager::instance().loadResource<ShaderData>("res/vertex.glsl");
    auto shader_frag = ResourceManager::instance().loadResource<ShaderData>("res/fragment.glsl");

    std::shared_ptr<Node> root = std::make_shared<Node>("root");
    root->addChild(std::make_shared<Node>("dynamic", mesh.get(), false));

    root->setScale(glm::vec3(2.f));

    TextureObject texture(*texture1.get());
    texture.bind(GL_TEXTURE0);

    ShaderObject shader(*shader_vertex.get(), *shader_frag.get());
    shader.use();
    shader.setInt("u_texture", 0);

    InputHandler inputHandler(window);

    glm::vec3 mov = glm::vec3(0.f);

    InputAxis::Axes["Horizontal"] = InputAxis(GLFW_KEY_D, GLFW_KEY_A, GLFW_KEY_RIGHT, GLFW_KEY_LEFT, &inputHandler);
    InputAxis::Axes["Vertical"] = InputAxis(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_UP, GLFW_KEY_DOWN, &inputHandler);

    auto movementInputVector = InputVector("Horizontal", "Vertical");

    root->children[0]->translate(glm::vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputHandler.update();

        // Simple update and render cycle
        root->render();

        mov = movementInputVector.getValue();

        root->children[0]->translate(mov / 60.f);
        root->rotate(glm::vec3(0.f, glm::radians(45.f / 60.f), 0.f));
        root->updateTransform();

        if (inputHandler.isKeyPressed(GLFW_KEY_P))
        {
            printf("Saved!\n");
            save(root);
        }

        mov = glm::vec3(0.f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    save_thread.join();
    ResourceManager::instance().clear();
    cleanup();
    return 0;
}