#include <Engine/Transform.h>
#include "Transform.h"

Transform::Transform()
    : position(glm::vec3(0.f)), rotation(glm::quat(glm::vec3(0.))), scale(glm::vec3(1.f)), localTransform(glm::mat4(1.f)), globalTransform(glm::mat4(1.f)), transformChanged(false)
{
}

void Transform::setPosition(const glm::vec3 &newPos)
{
    position = newPos;
    transformChanged = true;
}
void Transform::setScale(const glm::vec3 &newScale)
{
    scale = newScale;
    transformChanged = true;
}
void Transform::setRotation(const glm::quat &newRotation)
{
    rotation = newRotation;
    transformChanged = true;
}

void Transform::translate(const glm::vec3 &pos)
{
    setPosition(position + pos);
}

void Transform::rescale(const glm::vec3 &scl)
{
    setScale(scale * scl);
}

void Transform::rotate(const glm::vec3 &rot)
{
    setRotation(rotation * glm::quat(rot));
}

glm::vec3 Transform::getPosition() const
{
    return position;
}
glm::quat Transform::getRotation() const
{
    return rotation;
}
glm::vec3 Transform::getScale() const
{
    return scale;
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
    j = nlohmann::json();
    if (transform.position != glm::vec3(0.f))
        j += {"position", transform.position};
        
    if (transform.rotation != glm::quat(glm::vec3(0.f)))
        j += {"rotation", transform.rotation};

    if (transform.scale != glm::vec3(1.f))
        j += {"scale", transform.scale};
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
