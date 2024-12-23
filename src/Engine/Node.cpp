#include "Node.h"
#include <string>
#include <memory>
#include <iostream>

#include <glm/gtx/string_cast.hpp>
#include <mutex>

void glm::to_json(nlohmann::json &j, const glm::vec3 &v)
{
    j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}
void glm::from_json(const nlohmann::json &j, glm::vec3 &v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
}
void glm::to_json(nlohmann::json &j, const glm::quat &q)
{
    j = nlohmann::json{{"w", q.w}, {"x", q.x}, {"y", q.y}, {"z", q.z}};
}
void glm::from_json(const nlohmann::json &j, glm::quat &q)
{
    j.at("w").get_to(q.w);
    j.at("x").get_to(q.x);
    j.at("y").get_to(q.y);
    j.at("z").get_to(q.z);
}

void to_json(nlohmann::json &j, const std::shared_ptr<Node> &node)
{
    std::shared_lock<std::shared_mutex> lock(node->mutex_);
    j = nlohmann::json{
        {"name", node->name},
        {"position", node->position},
        {"rotation", node->rotation},
        {"scale", node->scale},
        {"isStatic", node->isStatic},
        {"meshName", node->meshName},
        {"children", node->children}};
}

void from_json(const nlohmann::json &j, const std::shared_ptr<Node> &node)
{
    std::unique_lock<std::shared_mutex> lock(node->mutex_);
    j.at("name").get_to(node->name);
    j.at("position").get_to(node->position);
    j.at("rotation").get_to(node->rotation);
    j.at("scale").get_to(node->scale);
    j.at("isStatic").get_to(node->isStatic);
    j.at("meshName").get_to(node->meshName);
    // Deserialize children - need to convert each child from JSON to std::shared_ptr<Node>
    if (j.contains("children"))
    {
        if (j["children"].is_array())
        {
            node->children.clear();
            for (const auto &child_json : j["children"])
            {
                auto child_node = std::make_shared<Node>();
                child_json.get_to(child_node);
                node->children.push_back(child_node);
                child_node->parent = node.get();
                child_node->transformChanged = true;
            }
        }
        else
        {
            node->children.clear(); // Ensure that if "children" is not an array, we don't crash.
        }
    }
}

Node::Node(const std::string &name, const std::string &mesh_name, bool is_static)
    : name(name), position(glm::vec3(0.f)), scale(glm::vec3(1.f)), rotation(glm::quat(glm::vec3(0.f))), parent(nullptr), meshName(mesh_name), isStatic(is_static), transformChanged(false)
{
}
void Node::addChild(const std::shared_ptr<Node> &child)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    child->parent = this;
    children.push_back(child);
}
void Node::updateTransform()
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (transformChanged)
    {
        transformMatrix = glm::mat4(1.f);
        // Apply parent's transformation (if any)
        if (parent)
        {
            transformMatrix *= parent->transformMatrix;
        }
        transformMatrix *= glm::translate(glm::mat4(1.0f), position) *
                          glm::mat4_cast(rotation) *
                          glm::scale(glm::mat4(1.0f), scale);
        transformChanged = false;
    }

    // Traverse all children
    for (auto &child : children)
    {
        child->updateTransform(); // Recursive call to update all children
    }
}
void Node::render(ShaderObject *shader) const
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!meshName.empty())
    {
        MeshObject::GetMeshObject(meshName)->render(shader, transformMatrix);
    }

    // Render children
    for (const auto &child : children)
    {
        child->render(shader);
    }
}
void Node::setPosition(const glm::vec3 &newPos)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    position = newPos;
    transformChanged = true;
}
void Node::setScale(const glm::vec3 &newScale)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    scale = newScale;
    transformChanged = true;
}
void Node::setRotation(const glm::quat &newRotation)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    rotation = newRotation;
    transformChanged = true;
}

void Node::translate(const glm::vec3 &pos)
{
    setPosition(position + pos);
}

void Node::rescale(const glm::vec3 &scl)
{
    setScale(scale * scl);
}

void Node::rotate(const glm::vec3 &rot)
{
    setRotation(rotation * glm::quat(rot));
}

glm::vec3 Node::getPosition() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return position;
}
glm::quat Node::getRotation() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return rotation;
}
glm::vec3 Node::getScale() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return scale;
}