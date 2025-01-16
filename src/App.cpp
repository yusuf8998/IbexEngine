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
#include "Graphics/Renderer.h"
#include "Engine/SkyboxNode.h"

std::thread save_thread;

void save(const std::shared_ptr<Node> &root)
{
    if (save_thread.joinable())
        save_thread.join();
    save_thread = std::thread(saveSceneGraph, "root.json", root);
    printf("Saved!\n");
}

// TODO: Skybox renderer assumes default DepthFunc. Create depth func field so it can be temporarily changed without assumption.

int main()
{
    auto &renderer = Renderer::instance();
    renderer.loadShader(0, "res/vertex_illum.glsl", "res/geometry_illum.glsl","res/fragment_illum.glsl");
    renderer.loadShader(1, "res/vertex_cube.glsl", "res/fragment_cube.glsl");
    renderer.assignSkyboxShader(1);

    // auto mesh = MeshObject("res/cube.obj");

    // NodePtr root = makeNode<Transformable>("root");
    // root->addChild(makeNode<Renderable>("dynamic1"));
    // root->addChild(makeNode<Renderable>("dynamic2"));
    // root->addChild(makeNode<Renderable>("dynamic3"));
    // root->addChild(makeNode<SkyboxNode>("skybox"));

    // castNode<Renderable>(root->children[0])->renderName = "res/cube2.obj";
    // castNode<Renderable>(root->children[1])->renderName = "res/cube2.obj";
    // castNode<Renderable>(root->children[2])->renderName = "res/cube.obj";
    // castNode<Renderable>(root->children[3])->renderName = "res/skybox*jpg";

    // castNode<Transformable>(root->children[0])->getTransform().position = glm::vec3(0, 1, 0);
    // castNode<Transformable>(root->children[1])->getTransform().position = glm::vec3(0, -1, 0);
    // castNode<Transformable>(root->children[2])->getTransform().position = glm::vec3(0, 0, 5);

    NodePtr root;
    loadSceneGraph("root.json", root);

    InputAxis::Axes["Horizontal"] = InputAxis(GLFW_KEY_D, GLFW_KEY_A, renderer.getInputHandler());
    InputAxis::Axes["Vertical"] = InputAxis(GLFW_KEY_S, GLFW_KEY_W, renderer.getInputHandler());
    InputAxis::Axes["Z"] = InputAxis(GLFW_KEY_E, GLFW_KEY_Q, renderer.getInputHandler());
    InputAxis::Axes["RotationVertical"] = InputAxis(GLFW_KEY_UP, GLFW_KEY_DOWN, renderer.getInputHandler());
    InputAxis::Axes["RotationHorizontal"] = InputAxis(GLFW_KEY_RIGHT, GLFW_KEY_LEFT, renderer.getInputHandler());

    auto movementInputVector = InputVector("Horizontal", "Z", "Vertical");
    auto rotationInputVector = InputVector("RotationHorizontal", "RotationVertical", "");

    glm::vec4 transformedInput;

    while (!renderer.shouldClose())
    {
        renderer.update();

        if (renderer.getInputHandler()->isKeyPressed(GLFW_KEY_P))
        {
            save(root);
        }

        if (renderer.getInputHandler()->isKeyPressed(GLFW_KEY_N))
        {
            castNode<Renderable>(root->children[0])->visible = !castNode<Renderable>(root->children[0])->visible;
        }

        if (renderer.getInputHandler()->isKeyPressed(GLFW_KEY_ESCAPE))
        {
            renderer.flipCursorState();
        }

        transformedInput = glm::vec4(movementInputVector.getValue(), 1.f);
        transformedInput = mainCamera.getRotationMatrix() * transformedInput;
        mainCamera.position += glm::vec3(transformedInput) * renderer.getDeltaTime();

        renderer.setViewProjectionUniforms();

        // Simple update and render cycle
        updateSceneGraph(root);
        renderSceneGraph(root, renderer.getShader(0));

        castNode<Transformable>(root)->getTransform().rotate(rotationInputVector.getValue() * renderer.getDeltaTime());

        renderer.postUpdate();
    }
    if (save_thread.joinable())
        save_thread.join();
    root.reset();
    MeshObject::ReleaseAllMeshes();
    ResourceManager::instance().clear();
    return 0;
}