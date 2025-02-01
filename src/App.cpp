#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

#include "Graphics/ShaderObject.h"
#include "Graphics/TextureObject.h"
#include "ResourceManager/ResourceManager.h"
#include "Graphics/RenderObject.h"
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

int main()
{
    auto &renderer = Renderer::instance();
    renderer.loadShader(0, "res/Shaders/Shader_Illum/vertex_illum.glsl", "res/Shaders/Shader_Illum/geometry_illum.glsl", "res/Shaders/Shader_Illum/fragment_illum.glsl");
    renderer.loadShader(1, "res/Shaders/Shader_Cube/vertex_cube.glsl", "res/Shaders/Shader_Cube/fragment_cube.glsl");
    renderer.assignSkyboxShader(1);

    NodePtr root = makeNode<Transformable>("root");
    root->addChild(makeNode<Renderable>("dynamic1"));
    root->addChild(makeNode<SkyboxNode>("skybox"));

    glm::mat4 model = glm::translate(glm::mat4(1.f), glm::vec3(0, 0, 1));

    // auto combined = MeshData::CombineMeshes(*RenderObject::GetRenderObject("res/Combine1.obj")->data, glm::mat4(1.f), *RenderObject::GetRenderObject("res/Combine2.obj")->data, glm::mat4(1.f));
    auto combined = MeshData::CombineMeshes({RenderObject::GetRenderObject("res/Combine2.obj")->data, RenderObject::GetRenderObject("res/Pebble_Sphere.obj")->data}, {glm::mat4(1.f), glm::mat4(1.f)});
    auto combinedobj = RenderObject::AddRenderObject("res/Combine2.obj+res/Pebble_Sphere.obj", std::make_shared<RenderObject>(combined));

    // auto combine2 = ResourceManager::instance().getResource<MeshData>("res/Combine2.obj");
    // combine2->applyTransformation(glm::translate(glm::mat4(1.f), glm::vec3(0, -1, 0)));

    // auto combine2obj = RenderObject::AddRenderObject(combine2->filepath, std::make_shared<RenderObject>(combine2));

    castNode<Renderable>(root->children[0])->renderName = "res/Combine2.obj+res/Pebble_Sphere.obj";
    castNode<SkyboxNode>(root->children[1])->renderName = "res/Textures/Skybox/skybox-biglake*jpg";

    // NodePtr root;
    // loadSceneGraph("root.json", root);

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
    RenderObject::ReleaseAllMeshes();
    ResourceManager::instance().clear();
    return 0;
}