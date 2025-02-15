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
#include "Engine/BillboardNode.h"
#include "Graphics/ParticleObject.h"

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
    renderer.loadShader(2, "res/Shaders/Shader_Normal/vertex_normal.glsl",  "res/Shaders/Shader_Normal/geometry_normal.glsl", "res/Shaders/Shader_Normal/fragment_normal.glsl");
    renderer.loadShader(3, "res/Shaders/Shader_Billboard/vertex_billboard.glsl",  "res/Shaders/Shader_Billboard/geometry_billboard.glsl", "res/Shaders/Shader_Billboard/fragment_billboard.glsl");
    renderer.loadShader(4, "res/Shaders/Shader_Displacement/vertex_displacement.glsl",  "res/Shaders/Shader_Displacement/geometry_displacement.glsl", "res/Shaders/Shader_Displacement/fragment_displacement.glsl");
    renderer.loadShader(5, "res/Shaders/Shader_Particle/vertex_particle.glsl", "res/Shaders/Shader_Particle/fragment_particle.glsl");
    renderer.assignSkyboxShader(1);

    ParticleObject particleObj("res/Textures/disp2.png");
    particleObj.particles = std::vector<Particle>(50);
    for (size_t i = 0; i < particleObj.particles.size(); i++) {
        particleObj.particles[i].position = glm::vec3(0.f, 5.f, -5.f);
        particleObj.particles[i].velocity = glm::normalize(glm::vec3(rand() % 10, rand() % 10, rand() % 10)) * 15.f;  // Random velocity
        particleObj.particles[i].acceleration = glm::vec3(.0f, -9.8f, .0f);
        particleObj.particles[i].size = .25f;
        particleObj.particles[i].color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);  // White color
        particleObj.particles[i].lifetime = 0.0f;
    }

    particleObj.updateInstanceBuffer();

    // NodePtr root = makeNode<Transformable>("root");
    // root->addChild(makeNode<Renderable>("dynamic1"));
    // root->addChild(makeNode<BillboardNode>("billboard1"));
    // root->addChild(makeNode<SkyboxNode>("skybox"));

    // // castNode<Transformable>(root)->is_static = true;

    // castNode<BillboardNode>(root->children[1])->transform.translate(glm::vec3(0, 5, 0));
    // castNode<BillboardNode>(root->children[1])->transform.rescale(glm::vec3(2.f));

    // castNode<Renderable>(root->children[0])->render_name = "res/Models/disp_cube.obj";
    // castNode<BillboardNode>(root->children[1])->render_name = "res/Textures/box.png";
    // castNode<SkyboxNode>(root->children[2])->render_name = "res/Textures/Skybox/skybox-biglake*jpg";

    // castNode<BillboardNode>(root->children[1])->lockHorizontal = true;

    NodePtr root;
    loadSceneGraph("root.json", root);

    InputAxis::Axes["Horizontal"] = InputAxis(GLFW_KEY_D, GLFW_KEY_A, renderer.getInputHandler());
    InputAxis::Axes["Vertical"] = InputAxis(GLFW_KEY_S, GLFW_KEY_W, renderer.getInputHandler());
    InputAxis::Axes["Z"] = InputAxis(GLFW_KEY_E, GLFW_KEY_Q, renderer.getInputHandler());
    InputAxis::Axes["RotationVertical"] = InputAxis(GLFW_KEY_UP, GLFW_KEY_DOWN, renderer.getInputHandler());
    InputAxis::Axes["RotationHorizontal"] = InputAxis(GLFW_KEY_RIGHT, GLFW_KEY_LEFT, renderer.getInputHandler());
    InputAxis::Axes["Sprint"] = InputAxis(GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL, renderer.getInputHandler());

    auto movementInputVector = InputVector("Horizontal", "Z", "Vertical");
    auto rotationInputVector = InputVector("RotationHorizontal", "RotationVertical", "");

    bool drawNormals = false;
    bool drawWireframe = false;
    bool updateParticles = false;

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
            drawNormals = !drawNormals;
        }

        if (renderer.getInputHandler()->isKeyPressed(GLFW_KEY_L))
        {
            drawWireframe = !drawWireframe;
        }

        if (renderer.getInputHandler()->isKeyPressed(GLFW_KEY_ESCAPE))
        {
            renderer.flipCursorState();
        }

        if (renderer.getInputHandler()->isKeyPressed(GLFW_KEY_F))
        {
            updateParticles = !updateParticles;
        }

        transformedInput = glm::vec4(movementInputVector.getValue(), 1.f);
        transformedInput = mainCamera.getRotationMatrix() * transformedInput;

        float sprint = renderer.getInputHandler()->getAxis("Sprint").getValue();
        float speedMult = sprint > 0.f ? sprint * 5.f : sprint < 0.f ? abs(sprint) * .5f : 1.f;
        transformedInput *= speedMult;

        mainCamera.position += glm::vec3(transformedInput) * renderer.getDeltaTime();

        renderer.setViewProjectionUniforms();

        // Simple update and render cycle
        updateSceneGraph(root);

        if (updateParticles)
        {
            particleObj.updateParticles();
            particleObj.updateInstanceBuffer();
        }
        particleObj.render(renderer.getShader(5), glm::mat4(1.f));

        glPolygonMode(GL_FRONT_AND_BACK, (drawWireframe ? GL_LINE : GL_FILL) );
        renderSceneGraph(root, renderer.getShader(4));
        if (drawNormals)
            renderSceneGraph(root, renderer.getShader(2), true);

        castNode<Transformable>(root)->transform.rotate(rotationInputVector.getValue() * renderer.getDeltaTime());

        renderer.postUpdate();
    }
    if (save_thread.joinable())
        save_thread.join();
    root.reset();
    RenderObject::ReleaseAllMeshes();
    ResourceManager::instance().clear();
    return 0;
}