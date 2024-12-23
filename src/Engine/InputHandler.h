#pragma once

#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <GLFW/glfw3.h>
#include <unordered_map>

#include <string>
#include <glm/glm.hpp>

class InputHandler;

struct InputAxis
{
    int positive_key, negative_key;
    int alt_positive_key, alt_negative_key;

    InputHandler *handler;

    InputAxis() = default;
    InputAxis(int pos_key, int neg_key, InputHandler *handler);
    InputAxis(int pos_key, int neg_key, int alt_pos_key, int alt_neg_key, InputHandler *handler);

    float getValue() const;

    static glm::vec3 GetVectorValue(const std::string &x_name, const std::string &y_name = "", const std::string &z_name = "");

    static std::unordered_map<std::string, InputAxis> Axes;
};

struct InputVector
{
    InputAxis *x, *y, *z;

    InputVector(InputAxis *_x, InputAxis *_y = 0, InputAxis *_z = 0);
    InputVector(const std::string &x_name, const std::string &y_name = "", const std::string &z_name = "");

    glm::vec3 getValue() const;
};

class InputHandler
{
public:
    explicit InputHandler(GLFWwindow *window);
    ~InputHandler();

    static InputHandler *instance();

    // Keyboard input functions
    bool isKeyPressed(int key) const;
    bool isKeyReleased(int key) const;
    bool isKeyHeld(int key) const;

    // Mouse input functions
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonReleased(int button) const;
    bool isMouseButtonHeld(int button) const;

    // Mouse movement functions
    void getMousePosition(double &x, double &y) const;

    void getMouseDelta(double &x, double &y) const;

    InputAxis getAxis(const std::string &name);

    // Update the state of the input system (to be called each frame)
    void update();

private:
    static InputHandler *instance_ptr;
    GLFWwindow *window;
    mutable std::unordered_map<int, bool> keyStates;             // Stores the current state of keys
    mutable std::unordered_map<int, bool> prevKeyStates;         // Stores the previous state of keys
    mutable std::unordered_map<int, bool> mouseButtonStates;     // Stores the current state of mouse buttons
    mutable std::unordered_map<int, bool> prevMouseButtonStates; // Stores the previous state of mouse buttons
    mutable double mouseX, mouseY;                               // Current mouse position
    mutable double prevMouseX, prevMouseY;                       // Previous mouse position

    // // Key callback
    // static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    // // Mouse button callback
    // static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    // // Cursor position callback
    // static void cursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
};

#endif