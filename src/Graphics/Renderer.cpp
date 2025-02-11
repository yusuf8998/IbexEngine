#include <Graphics/Renderer.h>
#include <Engine/Camera.h>
#include "Renderer.h"

#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/ShaderData.h>

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    mainCamera.processMouseScroll((float)yoffset);
}

Renderer::Renderer()
{
    initialize();
}

Renderer::~Renderer()
{
    shaders.clear();
    cleanup();
    delete inputHandler;
}

glm::uvec2 Renderer::getScreenSize() const
{
    std::shared_lock lock(mutex_);
    return screenSize;
}

InputHandler *Renderer::getInputHandler() const
{
    std::shared_lock lock(mutex_);
    return inputHandler;
}

GLFWwindow *Renderer::getWindow() const
{
    std::shared_lock lock(mutex_);
    return window;
}

int Renderer::getCursorState() const
{
    std::shared_lock lock(mutex_);
    return cursorState;
}

glm::vec3 Renderer::getClearColor() const
{
    return clearColor;
}

float Renderer::getDeltaTime() const
{
    return deltaTime;
}

glm::dvec2 Renderer::getDeltaMouse() const
{
    return deltaMouse;
}

std::shared_ptr<ShaderObject> Renderer::getShader(int key) const
{
    assert(shaders.find(key) != shaders.end());
    return shaders.at(key);
}

std::shared_ptr<ShaderObject> Renderer::getSkyboxShader() const
{
    return getShader(skyboxShader);
}

int Renderer::getSkyboxShaderIndex() const
{
    return skyboxShader;
}

void Renderer::setClearColor(const glm::vec3 &color)
{
    clearColor = color;
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
}

void Renderer::setCursorState(int state)
{
    std::unique_lock lock(mutex_);
    assert(state == GLFW_CURSOR_NORMAL || state == GLFW_CURSOR_DISABLED || state == GLFW_CURSOR_HIDDEN);
    cursorState = state;
    glfwSetInputMode(window, GLFW_CURSOR, cursorState);
}

void Renderer::setScreenSize(const glm::uvec2 &size)
{
    std::unique_lock lock(mutex_);
    screenSize = size;
    glfwSetWindowSize(window, size.x, size.y);
    glViewport(0, 0, size.x, size.y);
}

void Renderer::loadShader(int key, const std::string &vertexPath, const std::string &fragmentPath)
{
    assert(shaders.find(key) == shaders.end());
    std::unique_lock lock(mutex_);
    auto shader_vertex = ResourceManager::instance().loadResource<ShaderData>(vertexPath);
    auto shader_frag = ResourceManager::instance().loadResource<ShaderData>(fragmentPath);

    shaders[key] = std::make_shared<ShaderObject>(shader_vertex, shader_frag);
    shaders[key]->use();
}

void Renderer::loadShader(int key, const std::string &vertexPath, const std::string &geometryPath, const std::string &fragmentPath)
{
    assert(shaders.find(key) == shaders.end());
    std::unique_lock lock(mutex_);
    auto shader_vertex = ResourceManager::instance().loadResource<ShaderData>(vertexPath);
    auto shader_frag = ResourceManager::instance().loadResource<ShaderData>(fragmentPath);
    auto shader_geo = ResourceManager::instance().loadResource<ShaderData>(geometryPath);

    shaders[key] = std::make_shared<ShaderObject>(shader_vertex, shader_frag, shader_geo);
    shaders[key]->use();
}

void Renderer::assignSkyboxShader(int key)
{
    assert(shaders.count(key) != 0);
    skyboxShader = key;
}

void Renderer::unassignSkyboxShader()
{
    skyboxShader = -1;
}

void Renderer::setViewProjectionUniforms() const
{
    for (auto &kvp : shaders)
    {
        setViewProjectionUniforms(kvp.first);
    }
}

void Renderer::setViewProjectionUniforms(int key) const
{
    auto shader = getShader(key);
    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
}

bool Renderer::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Renderer::flipCursorState()
{
    setCursorState(cursorState == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Renderer::update()
{
    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    inputHandler->update();

    inputHandler->getMouseDelta(deltaMouse.x, deltaMouse.y);
    mainCamera.processMouseMovement(deltaMouse.x, deltaMouse.y);

    // Setup view and projection matrices
    view = mainCamera.getViewMatrix();
    projection = glm::perspective(glm::radians(mainCamera.zoom), (float)screenSize.x / screenSize.y, 0.1f, 100.f);
}

void Renderer::postUpdate() const
{
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Renderer::cleanup() const
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Renderer::initialize()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return;
    }

    window = glfwCreateWindow(screenSize.x, screenSize.y, windowTitle, nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable V-Sync
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, cursorState);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return;
    }

    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.f);
    glViewport(0, 0, screenSize.x, screenSize.y);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(DEFAULT_DEPTH_FUNC);
    glEnable(GL_MULTISAMPLE);  

    inputHandler = new InputHandler(window);
}