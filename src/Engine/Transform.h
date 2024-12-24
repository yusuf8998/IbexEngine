#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <nlohmann/json.hpp>
#include <Engine/GLMJsonfwd.h>

class Transform
{
public:
    glm::vec3 position; // Position of the node
    glm::quat rotation; // Rotation of the node
    glm::vec3 scale;    // Scale of the node
    glm::mat4 localTransform;
    glm::mat4 globalTransform;
    bool transformChanged;

    Transform();

    // Set the position of the node
    void setPosition(const glm::vec3 &newPos);
    // Set the scale of the node
    void setScale(const glm::vec3 &newScale);
    // Set the rotation of the node
    void setRotation(const glm::quat &newRotation);

    void translate(const glm::vec3 &pos);
    void rescale(const glm::vec3 &scl);
    void rotate(const glm::vec3 &rot);

    // Get the position (read-only)
    glm::vec3 getPosition() const;
    // Get the rotation (read-only)
    glm::quat getRotation() const;
    // Get the scale (read-only)
    glm::vec3 getScale() const;

    void applyTransformToLocal();
    void applyParentToGlobal(const glm::mat4 &parent);
    void applyLocalToGlobal();
};

void to_json(nlohmann::json &j, const Transform &transform);
void from_json(const nlohmann::json &j, Transform &transform);