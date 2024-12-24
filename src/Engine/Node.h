#pragma once

#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Graphics/MeshObject.h>
#include <Engine/Transform.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include <nlohmann/json.hpp>
#include <shared_mutex>

#include <Engine/GLMJsonfwd.h>

// Base class for nodes in the scene graph
class Node
{
public:
    std::string name;
    Transform transform;
    std::string meshName;
    bool isStatic;
    Node *parent;
    std::vector<std::shared_ptr<Node>> children;

    Node(const std::string &name = "Unnamed", const std::string &mesh_name = "", bool is_static = false);
    // Add a child node
    void addChild(const std::shared_ptr<Node> &child);
    // Apply transformations and build the final transformation matrix
    void updateTransform(bool keep_global = false);
    // Render (or any other operation that needs the transform)
    void render(ShaderObject *shader) const;

    // private:
    mutable std::shared_mutex mutex_; // Shared mutex to protect shared state
};

void to_json(nlohmann::json &j, const std::shared_ptr<Node> &node);
void from_json(const nlohmann::json &j, const std::shared_ptr<Node> &node);

inline void saveNodeToFile(const std::shared_ptr<Node> &rootNode, const std::string &filename)
{
    nlohmann::json j = rootNode;
    std::ofstream file(filename);
    file << j.dump(4); // pretty print with an indent of 4 spaces
    file.close();
}

inline std::shared_ptr<Node> loadNodeFromFile(const std::string &filename)
{
    std::ifstream file(filename);
    nlohmann::json j;
    file >> j;
    file.close();

    std::shared_ptr<Node> rootNode = std::make_shared<Node>();
    j.get_to(rootNode);
    rootNode->transform.transformChanged = true;
    rootNode->updateTransform();
    return rootNode;
}