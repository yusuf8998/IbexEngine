#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Mesh.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <shared_mutex>

// Base class for nodes in the scene graph
class Node
{
public:
    std::string name;
    glm::vec3 position; // Position of the node
    glm::quat rotation; // Rotation of the node
    glm::vec3 scale;    // Scale of the node
    glm::mat4 transformMatrix;
    Mesh *mesh;
    bool isStatic;
    bool transformChanged;
    Node *parent;
    std::vector<std::shared_ptr<Node>> children;

    Node(const std::string &name = "Unnamed", Mesh *_mesh = 0, bool is_static = false);
    // Add a child node
    void addChild(const std::shared_ptr<Node> &child);
    // Apply transformations and build the final transformation matrix
    void updateTransform();
    // Render (or any other operation that needs the transform)
    void render() const;
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

// private:
    mutable std::shared_mutex mutex_; // Shared mutex to protect shared state
};

namespace glm
{
    void to_json(nlohmann::json &j, const glm::vec3 &v);
    void from_json(const nlohmann::json &j, glm::vec3 &v);

    void to_json(nlohmann::json &j, const glm::quat &q);
    void from_json(const nlohmann::json &j, glm::quat &q);
}

void to_json(nlohmann::json &j, const std::shared_ptr<Node> &node);
void from_json(const nlohmann::json &j, const std::shared_ptr<Node> &node);

inline void saveNodeToFile(const std::shared_ptr<Node> &rootNode, const std::string &filename)
{
    nlohmann::json j = rootNode;
    std::ofstream file(filename);
    file << j.dump(4); // pretty print with an indent of 4 spaces
    file.close();
}