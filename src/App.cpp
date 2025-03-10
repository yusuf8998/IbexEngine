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
#include "Graphics/InputManager/InputManager.h"
#include "Engine/Camera.h"
#include <thread>
#include "Graphics/Renderer.h"
#include "Engine/SkyboxNode.h"
#include "Engine/BillboardNode.h"
#include "Graphics/ParticleObject.h"
#include "Engine/FSM/PlayerMachine.h"

#include <Engine/Editor/Inspector.h>
#include "EventClock.h"

#include <Engine/LightNode.h>
#include "glm/gtx/string_cast.hpp"
#include "Graphics/FramebufferObject.h"

std::thread save_thread;

void save(const std::shared_ptr<Node> &root)
{
    if (save_thread.joinable())
        save_thread.join();
    save_thread = std::thread(saveSceneGraph, "res/root.json", root);
    printf("Saved!\n");
}

// Lighting doesn't work with displacement maps
// Some vectors are wrong way
// Displacement doesn't use normal map

int main()
{
    auto &renderer = Renderer::instance();
    auto &input = InputManager::instance();
    renderer.loadShader(0, "res/Shaders/Shader_Illum");
    renderer.loadShader(1, "res/Shaders/Shader_Cube");
    renderer.loadShader(2, "res/Shaders/Shader_Normal");
    renderer.loadShader(3, "res/Shaders/Shader_Billboard");
    renderer.loadShader(4, "res/Shaders/Shader_Displacement");
    renderer.loadShader(5, "res/Shaders/Shader_Particle");
    renderer.loadShader(6, "res/Shaders/Shader_Depth");
    renderer.assignSkyboxShader(1);

    std::vector<Particle> particles = std::vector<Particle>(25);
    for (size_t i = 0; i < particles.size(); i++)
    {
        particles[i].position = glm::vec3(0.f, 5.f, -5.f);
        particles[i].velocity = glm::normalize(glm::vec3(rand() % 10 - 5, rand() % 3 - 1, rand() % 10 - 5)) * ((rand() % 151) / 10.f); // Random velocity
        particles[i].acceleration = glm::vec3(.0f, -9.8f, .0f);
        particles[i].size = .25f;
        particles[i].color = glm::vec4(1.0f, 0.125f, 0.0f, 1.0f);
        particles[i].lifetime = 0.0f;
    }
    ParticleObject particleObj("res/Textures/disp2.png", particles);

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
    loadSceneGraph("res/root.json", root);

    // auto lightContainer = makeNode<Node>("LightContainer");
    // root->addChild(lightContainer);

    // auto dirLight = makeNode<LightNode>("DirLight");
    // dirLight->setCaster(std::make_shared<DirectionalLight>(LightColor{glm::vec3(0.0125f), glm::vec3(0.5f), glm::vec3(0.5f)}));
    // dirLight->transform.setLocalRotation(glm::quatLookAt(glm::vec3(0, -1, 0), glm::vec3(0, 0, -1)));
    // dirLight->setActive(true);
    // lightContainer->addChild(dirLight);

    // auto pointLight = makeNode<LightNode>("PointLight");
    // glm::vec3 green(0.0125f, 1.f, 0.0125f);
    // pointLight->setCaster(std::make_shared<PointLight>(LightColor{glm::vec3(0.0125f) * green, glm::vec3(0.5f) * green, glm::vec3(0.5f) * green}, LightAttenuation{1.f, 0.55f, 0.44f}));
    // pointLight->transform.setLocalPosition(glm::vec3(0, 1.2f, 0));
    // pointLight->setActive(true);
    // lightContainer->addChild(pointLight);

    InputAxis::Axes["Horizontal"] = std::make_shared<InputAxis>(GLFW_KEY_D, GLFW_KEY_A);
    InputAxis::Axes["Vertical"] = std::make_shared<InputAxis>(GLFW_KEY_S, GLFW_KEY_W);
    InputAxis::Axes["Z"] = std::make_shared<InputAxis>(GLFW_KEY_E, GLFW_KEY_Q);
    InputAxis::Axes["RotationVertical"] = std::make_shared<InputAxis>(GLFW_KEY_UP, GLFW_KEY_DOWN);
    InputAxis::Axes["RotationHorizontal"] = std::make_shared<InputAxis>(GLFW_KEY_RIGHT, GLFW_KEY_LEFT);
    InputAxis::Axes["Sprint"] = std::make_shared<InputAxis>(GLFW_KEY_LEFT_SHIFT, GLFW_KEY_LEFT_CONTROL);

    auto movementInputVector = InputVector("Horizontal", "Z", "Vertical");
    auto rotationInputVector = InputVector("RotationHorizontal", "RotationVertical", "");

    PlayerEvent playerEvent;
    tinyfsm::FsmList<PlayerMachine>::start();

    bool drawNormals = false;
    bool drawWireframe = false;
    bool updateParticles = false;
    bool mouseLook = true;

    glm::vec4 transformedInput;

    (void)ImGui::CreateContext();
    (void)ImGui_ImplGlfw_InitForOpenGL(renderer.getWindow(), true);
    (void)ImGui_ImplOpenGL3_Init("#version 450");
    ImGui::StyleColorsDark();

    EventClock purgeClock(5.f, []()
                          {  RenderObject::Purge();
                        ResourceManager::instance().purgeAll(); }, [](float ct, float dt)
                          { return ct + dt; });

    FramebufferObject fbo(renderer.getScreenSize().x, renderer.getScreenSize().y, false, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    bool bind_fbo = false;

    while (!renderer.shouldClose())
    {
        renderer.update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (input.isKeyPressed(GLFW_KEY_F1))
            save(root);
        if (input.isKeyPressed(GLFW_KEY_F2))
            drawNormals = !drawNormals;
        if (input.isKeyPressed(GLFW_KEY_F3))
            drawWireframe = !drawWireframe;
        if (input.isKeyPressed(GLFW_KEY_ESCAPE))
            renderer.flipCursorState();
        if (input.isKeyPressed(GLFW_KEY_F4))
            updateParticles = !updateParticles;
        if (input.isKeyPressed(GLFW_KEY_F5))
            mouseLook = !mouseLook;
        if (input.isKeyPressed(GLFW_KEY_F6))
            root->traverse([&](Node *node)
                           { if (auto *cast = dynamic_cast<Renderable *>(node)) { cast->reset(); } });
        if (input.isKeyPressed(GLFW_KEY_F7))
            RenderObject::DebugUseCounts();
        if (input.isKeyPressed(GLFW_KEY_F8))
            ResourceManager::instance().debugUseCounts();
        if (input.isKeyPressed(GLFW_KEY_F9))
            bind_fbo = !bind_fbo;

        if (mouseLook)
        {
            mainCamera.processMouseMovement(renderer.getDeltaMouse().x, renderer.getDeltaMouse().y);
            transformedInput = glm::vec4(movementInputVector.getValue(), 1.f);
            transformedInput = mainCamera.getRotationMatrix() * transformedInput;
            playerEvent.direction = glm::vec3(transformedInput);
            castNode<Transformable>(root)->transform.rotate(rotationInputVector.getValue() * renderer.getDeltaTime());
            sendPlayerEvent(playerEvent);
        }

        renderer.setViewProjectionUniforms();

        // Simple update and render cycle
        updateSceneGraph(root);

        LightNode::UpdateActiveLights();
        LightNode::UpdateActiveLightVectors();

        if (updateParticles)
        {
            particleObj.updateParticles();
            particleObj.updateInstanceBuffer();
        }
        if (bind_fbo)
        {
            fbo.bindTexture();
            fbo.bind();
            // renderSceneGraph(root, renderer.getShader(6), true);
            // fbo.unbind();
        }
        else
        {
            fbo.unbind();
        }
        LightNode::RenderDepthMaps(renderer.getShader(6), [&]()
                                   { renderSceneGraph(root, renderer.getShader(6), true); });
        glPolygonMode(GL_FRONT_AND_BACK, (drawWireframe ? GL_LINE : GL_FILL));
        renderer.resetViewport();
        particleObj.render(renderer.getShader(5), glm::mat4(1.f));
        renderSceneGraph(root, renderer.getShader(4), false);
        if (drawNormals)
            renderSceneGraph(root, renderer.getShader(2), true);

        {
            {
                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                (void)ImGui::Text("Delta Time: %f s", renderer.getDeltaTime());

                Inspector::drawNode(root);
            }
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        purgeClock.update(renderer.getDeltaTime());

        renderer.postUpdate();
    }
    tinyfsm::FsmList<PlayerMachine>::reset();
    if (save_thread.joinable())
        save_thread.join();
    root.reset();
    RenderObject::ReleaseAllMeshes();
    ResourceManager::instance().clear();
    return 0;
}