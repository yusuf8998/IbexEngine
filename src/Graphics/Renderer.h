#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <shared_mutex>

#include <iostream>

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Graphics/ShaderObject.h>

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

constexpr GLenum DEFAULT_DEPTH_FUNC = GL_LESS;

class Renderer
{
private:
    glm::uvec2 screenSize = {1280, 720};
    const char *windowTitle = "Ibex Engine";
    GLFWwindow *window;

    glm::vec3 clearColor = {0.2f, 0.3f, 0.3f};

    int cursorState = GLFW_CURSOR_NORMAL;
    float deltaTime = 0.f;
    float lastFrame = 0.f;
    float currentFrame = 0.f;

    glm::mat4 projection, view;

    glm::dvec2 deltaMouse;

    mutable std::shared_mutex mutex_;

    std::map<int, std::shared_ptr<ShaderObject>> shaders;
    int skyboxShader = -1;

public:
    Renderer();
    ~Renderer();

    static Renderer &instance()
    {
        static Renderer instance;
        return instance;
    }

    void initialize();

    glm::uvec2 getScreenSize() const;
    GLFWwindow *getWindow() const;
    glm::vec3 getClearColor() const;
    int getCursorState() const;
    float getDeltaTime() const;
    glm::dvec2 getDeltaMouse() const;

    std::shared_ptr<ShaderObject> getShader(int key) const;
    std::shared_ptr<ShaderObject> getSkyboxShader() const;
    int getSkyboxShaderIndex() const;

    void setClearColor(const glm::vec3 &color);
    void setCursorState(int state);
    void setScreenSize(const glm::uvec2 &size);

    void loadShader(int key, const std::string &vertexPath, const std::string &fragmentPath);
    void loadShader(int key, const std::string &vertexPath, const std::string &geometryPath, const std::string &fragmentPath);

    void assignSkyboxShader(int key);
    void unassignSkyboxShader();

    void setViewProjectionUniforms() const;
    void setViewProjectionUniforms(int key) const;

    void flipCursorState();

    bool shouldClose() const;

    void update();
    void postUpdate() const;

    void cleanup() const;
};