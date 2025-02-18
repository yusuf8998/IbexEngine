#pragma once

#include <GLFW/glfw3.h>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <glm/glm.hpp>

struct InputAxis
{
    int positive_key, negative_key, alt_positive_key, alt_negative_key;
    float value;

    InputAxis() = default;
    InputAxis(int pos_key, int neg_key);
    InputAxis(int pos_key, int neg_key, int alt_pos_key, int alt_neg_key);

    static std::map<std::string, std::shared_ptr<InputAxis>> Axes;
};

struct InputVector
{
    std::shared_ptr<InputAxis> x, y, z;

    InputVector(const std::string &x_name, const std::string &y_name = "", const std::string &z_name = "");

    glm::vec3 getValue() const;
};


class InputManager
{
public:
    explicit InputManager() {}
    ~InputManager() {}

    static InputManager &instance()
    {
        static InputManager _instance;
        return _instance;
    }

    void setWindow(GLFWwindow *_window);

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

    float getAxis(const std::string &name) const;

    void update();

private:
    GLFWwindow *window;
    mutable std::map<int, bool> keyStates;             // Stores the current state of keys
    mutable std::map<int, bool> prevKeyStates;         // Stores the previous state of keys
    mutable std::map<int, bool> mouseButtonStates;     // Stores the current state of mouse buttons
    mutable std::map<int, bool> prevMouseButtonStates; // Stores the previous state of mouse buttons
    mutable double mouseX, mouseY;                     // Current mouse position
    mutable double prevMouseX, prevMouseY;             // Previous mouse position
};