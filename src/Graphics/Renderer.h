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

#include <Engine/InputHandler.h>

#include <Graphics/ShaderObject.h>

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

class Renderer
{
private:
    glm::uvec2 screen_size = {800, 600};
    const char *windowTitle = "Ibex Engine";
    GLFWwindow *window;

    InputHandler *inputHandler;

    int cursorState = GLFW_CURSOR_NORMAL;
    float deltaTime = 0.f;
    float lastFrame = 0.f;
    float currentFrame = 0.f;

    glm::dvec2 deltaMouse;

    mutable std::shared_mutex mutex_;

public:
    Renderer();
    ~Renderer();

    void initialize();

    glm::uvec2 getScreenSize() const;
    InputHandler *getInputHandler() const;
    GLFWwindow *getWindow() const;
    int getCursorState() const;
    float getDeltaTime() const;
    glm::dvec2 getDeltaMouse() const;

    void setCursorState(int state);
    void setScreenSize(const glm::uvec2 &size);

    void flipCursorState();

    bool shouldClose() const;

    void update();
    void postUpdate() const;

    void cleanup() const;
};