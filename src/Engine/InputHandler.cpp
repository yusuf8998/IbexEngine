#include "InputHandler.h"
#include <iostream>

InputHandler *InputHandler::instance_ptr = 0;

std::unordered_map<std::string, InputAxis> InputAxis::Axes = {};

InputAxis::InputAxis(int pos_key, int neg_key, InputHandler *handler)
    : InputAxis(pos_key, neg_key, 0, 0, handler)
{
}

InputAxis::InputAxis(int pos_key, int neg_key, int alt_pos_key, int alt_neg_key, InputHandler *handler)
    : positive_key(pos_key), negative_key(neg_key), handler(handler),
      alt_positive_key(alt_pos_key), alt_negative_key(alt_neg_key)
{
}

float InputAxis::getValue() const
{
    float axis = 0.f;
    if (handler->isKeyHeld(positive_key) || (alt_positive_key != 0 && handler->isKeyHeld(alt_positive_key)))
        axis += 1.f;
    if (handler->isKeyHeld(negative_key) || (alt_negative_key != 0 && handler->isKeyHeld(alt_negative_key)))
        axis -= 1.f;
    return axis;
}

glm::vec3 InputAxis::GetVectorValue(const std::string &x_name, const std::string &y_name, const std::string &z_name)
{
    float x_val = x_name == "" ? 0.f : Axes[x_name].getValue();
    float y_val = y_name == "" ? 0.f : Axes[y_name].getValue();
    float z_val = z_name == "" ? 0.f : Axes[z_name].getValue();
    glm::vec3 v = glm::vec3(x_val, y_val, z_val);
    if (glm::length(v) > 1)
        v = glm::normalize(v);
    return v;
}

InputVector::InputVector(InputAxis *_x, InputAxis *_y, InputAxis *_z)
    : x(_x), y(_y), z(_z)
{
}

InputVector::InputVector(const std::string &x_name, const std::string &y_name, const std::string &z_name)
{
    x = InputAxis::Axes.count(x_name) == 0 ? 0 : &InputAxis::Axes[x_name];
    y = InputAxis::Axes.count(y_name) == 0 ? 0 : &InputAxis::Axes[y_name];
    z = InputAxis::Axes.count(z_name) == 0 ? 0 : &InputAxis::Axes[z_name];
}

glm::vec3 InputVector::getValue() const
{
    float x_val = !x ? 0.f : x->getValue();
    float y_val = !y ? 0.f : y->getValue();
    float z_val = !z ? 0.f : z->getValue();
    glm::vec3 v = glm::vec3(x_val, y_val, z_val);
    if (glm::length(v) > 1)
        v = glm::normalize(v);
    return v;
}

InputHandler::InputHandler(GLFWwindow *window)
    : window(window), mouseX(0), mouseY(0), prevMouseX(0), prevMouseY(0)
{
    instance_ptr = this;
    // Set the GLFW callbacks
    // glfwSetKeyCallback(window, keyCallback);
    // glfwSetMouseButtonCallback(window, mouseButtonCallback);
    // glfwSetCursorPosCallback(window, cursorPositionCallback);

    // Initialize key states to false
    for (int i = 0; i < GLFW_KEY_LAST; i++)
    {
        keyStates[i] = false;
        prevKeyStates[i] = false;
    }

    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
    {
        mouseButtonStates[i] = false;
        prevMouseButtonStates[i] = false;
    }
}

InputHandler::~InputHandler() {}

InputHandler *InputHandler::instance()
{
    return instance_ptr;
}

void InputHandler::update()
{
    // Update the state of the keys and mouse buttons
    for (int i = 0; i < GLFW_KEY_LAST; i++)
    {
        prevKeyStates[i] = keyStates[i];
        keyStates[i] = glfwGetKey(window, i) == GLFW_PRESS;
    }

    for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; i++)
    {
        prevMouseButtonStates[i] = mouseButtonStates[i];
        mouseButtonStates[i] = glfwGetMouseButton(window, i) == GLFW_PRESS;
    }

    // Update mouse position
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

bool InputHandler::isKeyPressed(int key) const
{
    return keyStates.at(key) && !prevKeyStates.at(key);
}

bool InputHandler::isKeyReleased(int key) const
{
    return !keyStates.at(key) && prevKeyStates.at(key);
}

bool InputHandler::isKeyHeld(int key) const
{
    return keyStates.at(key);
}

bool InputHandler::isMouseButtonPressed(int button) const
{
    return mouseButtonStates.at(button) && !prevMouseButtonStates.at(button);
}

bool InputHandler::isMouseButtonReleased(int button) const
{
    return !mouseButtonStates.at(button) && prevMouseButtonStates.at(button);
}

bool InputHandler::isMouseButtonHeld(int button) const
{
    return mouseButtonStates.at(button);
}

void InputHandler::getMousePosition(double &x, double &y) const
{
    x = mouseX;
    y = mouseY;
}

void InputHandler::getMouseDelta(double &x, double &y) const
{
    x = mouseX - prevMouseX;
    y = mouseY - prevMouseY;
}

InputAxis InputHandler::getAxis(const std::string &name)
{
    return InputAxis::Axes[name];
}

// GLFW callbacks (static)
// void InputHandler::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
// {
//     // InputHandler *input = InputHandler::instance();
//     // input->update();
// }

// void InputHandler::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
// {
//     // InputHandler *input = InputHandler::instance();
//     // input->update();
// }

// void InputHandler::cursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
// {
//     // InputHandler *input = InputHandler::instance();
//     // input->update();
// }
