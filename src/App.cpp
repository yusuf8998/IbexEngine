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
    if (save_thread.joinable())
        save_thread.join();
    save_thread = std::thread(saveNodeToFile, root, "root.json");
}

int main()
{
    if (!initializeWindow())
    {
        return -1;
    }

    std::shared_ptr<MeshData> mesh = std::make_shared<MeshData>();
    mesh->loadFromOBJ("res/cube.obj");
    mesh->loadMaterialsFromMTL("res/cube.mtl");

    MeshObject *mesh_obj = new MeshObject(mesh.get());
    mesh_obj->generateOpenGLBuffers();

    auto texture1 = ResourceManager::instance().loadResource<TextureData>("res/box.png");

    auto shader_vertex = ResourceManager::instance().loadResource<ShaderData>("res/vertex_mesh.glsl");
    auto shader_frag = ResourceManager::instance().loadResource<ShaderData>("res/fragment_mesh.glsl");

    std::shared_ptr<Node> root = std::make_shared<Node>("root");
    root->addChild(std::make_shared<Node>("dynamic", mesh_obj, false));

    TextureObject texture = TextureObject(*texture1.get());
    texture.bind(GL_TEXTURE0);

    ShaderObject shader(*shader_vertex.get(), *shader_frag.get());
    shader.use();

    InputHandler inputHandler(window);

    glm::mat4 projection = glm::perspective(glm::radians(60.f), 800.f / 600.f, 0.125f, 10.f);//glm::ortho(-1, 1, -1, 1);

    glm::vec3 mov = glm::vec3(0.f);

    InputAxis::Axes["Horizontal"] = InputAxis(GLFW_KEY_D, GLFW_KEY_A, GLFW_KEY_RIGHT, GLFW_KEY_LEFT, &inputHandler);
    InputAxis::Axes["Vertical"] = InputAxis(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_UP, GLFW_KEY_DOWN, &inputHandler);
    InputAxis::Axes["Z"] = InputAxis(GLFW_KEY_Q, GLFW_KEY_E, &inputHandler);;

    auto movementInputVector = InputVector("Horizontal", "Vertical", "Z");

    // root->children[0]->translate(glm::vec3(0, 1, 0));

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputHandler.update();

        shader.setMat4("projection", projection);

        // Simple update and render cycle
        root->render(&shader);

        mov = movementInputVector.getValue();

        root->children[0]->translate(mov / 60.f);
        // root->rotate(glm::vec3(0.f, glm::radians(45.f / 60.f), 0.f));
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
    if (save_thread.joinable())
        save_thread.join();
    delete mesh_obj;
    ResourceManager::instance().clear();
    cleanup();
    return 0;
}