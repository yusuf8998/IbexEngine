#include <Graphics/InputManager/InputManager.h>

std::map<std::string, std::shared_ptr<InputAxis>> InputAxis::Axes;

InputAxis::InputAxis(int pos_key, int neg_key)
    : InputAxis(pos_key, neg_key, 0, 0)
{
}

InputAxis::InputAxis(int pos_key, int neg_key, int alt_pos_key, int alt_neg_key)
    : positive_key(pos_key), negative_key(neg_key), alt_positive_key(alt_pos_key), alt_negative_key(alt_neg_key)
{
    value = 0.f;
}

InputVector::InputVector(const std::string &x_name, const std::string &y_name, const std::string &z_name)
{
    x = InputAxis::Axes.count(x_name) == 0 ? 0 : InputAxis::Axes[x_name];
    y = InputAxis::Axes.count(y_name) == 0 ? 0 : InputAxis::Axes[y_name];
    z = InputAxis::Axes.count(z_name) == 0 ? 0 : InputAxis::Axes[z_name];
}
glm::vec3 InputVector::getValue() const
{
    float x_val = !x ? 0.f : x->value;
    float y_val = !y ? 0.f : y->value;
    float z_val = !z ? 0.f : z->value;
    glm::vec3 v = glm::vec3(x_val, y_val, z_val);
    if (glm::length(v) > 1)
        v = glm::normalize(v);
    return v;
}

void InputManager::setWindow(GLFWwindow *_window)
{
    window = _window;
    mouseX = 0;
    mouseY = 0;
    prevMouseX = 0;
    prevMouseY = 0;
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

float InputManager::getAxis(const std::string &name) const
{
    return InputAxis::Axes.at(name)->value;
}

void InputManager::update()
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

    for (auto &kvp : InputAxis::Axes)
    {
        auto &axis = kvp.second;
        float value = 0.f;
        if (isKeyHeld(axis->positive_key) || (axis->alt_positive_key != 0 && isKeyHeld(axis->alt_positive_key)))
            value += 1.f;
        if (isKeyHeld(axis->negative_key) || (axis->alt_negative_key != 0 && isKeyHeld(axis->alt_negative_key)))
            value -= 1.f;
        axis->value = value;
    }

    // Update mouse position
    prevMouseX = mouseX;
    prevMouseY = mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

bool InputManager::isKeyPressed(int key) const
{
    return keyStates.at(key) && !prevKeyStates.at(key);
}

bool InputManager::isKeyReleased(int key) const
{
    return !keyStates.at(key) && prevKeyStates.at(key);
}

bool InputManager::isKeyHeld(int key) const
{
    return keyStates.at(key);
}

bool InputManager::isMouseButtonPressed(int button) const
{
    return mouseButtonStates.at(button) && !prevMouseButtonStates.at(button);
}

bool InputManager::isMouseButtonReleased(int button) const
{
    return !mouseButtonStates.at(button) && prevMouseButtonStates.at(button);
}

bool InputManager::isMouseButtonHeld(int button) const
{
    return mouseButtonStates.at(button);
}

void InputManager::getMousePosition(double &x, double &y) const
{
    x = mouseX;
    y = mouseY;
}

void InputManager::getMouseDelta(double &x, double &y) const
{
    x = mouseX - prevMouseX;
    y = mouseY - prevMouseY;
}