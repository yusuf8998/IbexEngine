#include <Engine/Transform.h>
#include "Transform.h"

Transform::Transform()
    : position(glm::vec3(0.f)), rotation(glm::quat(glm::vec3(0.))), scale(glm::vec3(1.f)), localTransform(glm::mat4(1.f)), globalTransform(glm::mat4(1.f)), transformChanged(false)
{
}

void Transform::setLocalPosition(const glm::vec3 &newPos)
{
    position = newPos;
    transformChanged = true;
}
void Transform::setLocalScale(const glm::vec3 &newScale)
{
    scale = newScale;
    transformChanged = true;
}
void Transform::setLocalRotation(const glm::quat &newRotation)
{
    rotation = newRotation;
    transformChanged = true;
}

void Transform::translate(const glm::vec3 &pos)
{
    setLocalPosition(position + pos);
}

void Transform::rescale(const glm::vec3 &scl)
{
    setLocalScale(scale * scl);
}

void Transform::rotate(const glm::vec3 &rot)
{
    setLocalRotation(rotation * glm::quat(rot));
}

glm::vec3 Transform::getLocalPosition() const
{
    return position;
}
glm::quat Transform::getLocalRotation() const
{
    return rotation;
}
glm::vec3 Transform::getLocalScale() const
{
    return scale;
}

glm::vec3 Transform::getLocalFront() const
{
    return glm::normalize(glm::vec3(glm::mat3_cast(rotation) * glm::vec3(0.f, 0.f, -1.f)));
}

glm::vec3 Transform::getLocalRight() const
{
    return glm::normalize(glm::vec3(glm::mat3_cast(rotation) * glm::vec3(1.f, 0.f, 0.f)));
}

glm::vec3 Transform::getLocalUp() const
{
    return glm::normalize(glm::vec3(glm::mat3_cast(rotation) * glm::vec3(0.f, 1.f, 0.f)));
}

glm::vec3 Transform::getGlobalPosition() const
{
    return glm::vec3(globalTransform[3]);
}

glm::quat Transform::getGlobalRotation() const
{
    return glm::quat_cast(globalTransform);
}

glm::vec3 Transform::getGlobalScale() const
{
    return glm::vec3(glm::length(glm::vec3(globalTransform[0])), glm::length(glm::vec3(globalTransform[1])), glm::length(glm::vec3(globalTransform[2])));
}

glm::vec3 Transform::getGlobalFront() const
{
    return -glm::vec3(globalTransform[2]);
}

glm::vec3 Transform::getGlobalRight() const
{
    return -glm::vec3(globalTransform[0]);
}

glm::vec3 Transform::getGlobalUp() const
{
    return glm::vec3(globalTransform[1]);
}

void Transform::applyTransformToLocal()
{
    localTransform = glm::mat4(1.f);
    localTransform *= glm::translate(glm::mat4(1.0f), position) *
                      glm::mat4_cast(rotation) *
                      glm::scale(glm::mat4(1.0f), scale);
}

void Transform::applyParentToGlobal(const glm::mat4 &parent)
{
    globalTransform = glm::mat4(1.f);
    globalTransform *= parent;
}

void Transform::applyLocalToGlobal()
{
    globalTransform *= localTransform;
}

void to_json(nlohmann::json &j, const Transform &transform)
{
    j = nlohmann::json{
        {"position", transform.position},
        {"rotation", transform.rotation},
        {"scale", transform.scale}};
}

void from_json(const nlohmann::json &j, Transform &transform)
{
    if (j.contains("position"))
        j.at("position").get_to(transform.position);
    else
        transform.position = glm::vec3(0.f);

    if (j.contains("rotation"))
        j.at("rotation").get_to(transform.rotation);
    else
        transform.rotation = glm::quat(glm::vec3(0.f));

    if (j.contains("scale"))
        j.at("scale").get_to(transform.scale);
    else
        transform.scale = glm::vec3(1.f);
}
