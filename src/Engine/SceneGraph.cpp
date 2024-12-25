#include "SceneGraph.h"
#include <Graphics/MeshObject.h>

using namespace std;
using namespace glm;
using namespace nlohmann;

void glm::to_json(json &j, const vec3 &v)
{
    j = json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
}
void glm::from_json(const json &j, vec3 &v)
{
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
}
void glm::to_json(json &j, const quat &q)
{
    j = json{{"w", q.w}, {"x", q.x}, {"y", q.y}, {"z", q.z}};
}
void glm::from_json(const json &j, quat &q)
{
    j.at("w").get_to(q.w);
    j.at("x").get_to(q.x);
    j.at("y").get_to(q.y);
    j.at("z").get_to(q.z);
}

Node::Node(const string &name)
    : name(name), parent(nullptr), children()
{
}

void Node::addChild(NodePtr child)
{
    children.push_back(child);
    child->parent = this;
}

void to_json(json &j, const NodePtr &node)
{
    j = json{
        {"name", node->name},
        {"children", json::array()}};

    for (const auto &child : node->children)
    {
        j["children"].push_back(child);
    }

    if (auto *renderable = dynamic_cast<Renderable *>(node.get()))
    {
        ::to_json(j, renderable);
    }
    else if (auto *transformable = dynamic_cast<Transformable *>(node.get()))
    {
        ::to_json(j, transformable);
    }
}
void from_json(const json &j, NodePtr &node)
{
    constructNodeFromJson(j, node);
    j.at("name").get_to(node->name);
    // Deserialize children - need to convert each child from JSON to NodePtr
    if (j.contains("children") && j["children"].is_array())
    {
        node->children.clear();
        for (const auto &child_json : j["children"])
        {
            NodePtr child_node;
            constructNodeFromJson(child_json, child_node);
            ::from_json(child_json, child_node);
            node->children.push_back(child_node);
            child_node->parent = node.get();
        }
    }
}

void constructNodeFromJson(const json &j, NodePtr &node)
{
    if (j.contains("meshName"))
    {
        node = make_shared<Renderable>();
        ::from_json(j, dynamic_pointer_cast<Renderable>(node));
        return;
    }
    if (j.contains("transform"))
    {
        node = make_shared<Transformable>();
        ::from_json(j, dynamic_pointer_cast<Transformable>(node));
        return;
    }
    node = make_shared<Node>();
    return;
}

void Transformable::updateTransform(bool keep_global)
{
    if (transform.transformChanged)
    {
        transform.applyTransformToLocal();
    }

    if (!keep_global)
    {
        transform.globalTransform = mat4(1.f);
        if (auto *parent_cast = dynamic_cast<Transformable *>(parent))
        {
            transform.applyParentToGlobal(parent_cast->transform.globalTransform);
        }
        transform.applyLocalToGlobal();
    }

    // Traverse children

    transform.transformChanged = false;
}
void to_json(json &j, const TransformablePtr &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const Transformable *node)
{
    j += {"transform", node->getTransform()};
}
void from_json(const json &j, const TransformablePtr &node)
{
    j.at("transform").get_to(node->getTransform());
    node->getTransform().transformChanged = true;
    node->updateTransform();
}

void Renderable::render(ShaderObject *shader)
{
    // Render the mesh
    MeshObject::GetMeshObject(meshName)->render(shader, transform.globalTransform);
}
void to_json(json &j, const RenderablePtr &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const Renderable *node)
{
    ::to_json(j, dynamic_cast<const Transformable *>(node));
    j += {"meshName", node->meshName};
    j += {"visible", node->visible};
    j += {"static", node->static_};
}
void from_json(const json &j, const RenderablePtr &node)
{
    ::from_json(j, dynamic_pointer_cast<Transformable>(node));
    j.at("meshName").get_to(node->meshName);
    j.at("visible").get_to(node->visible);
    j.at("static").get_to(node->static_);
}

void loadSceneGraph(const string &filename, NodePtr &root)
{
    ifstream file(filename);
    json j;
    file >> j;
    file.close();
    j.get_to(root);
}
void saveSceneGraph(const string &filename, const NodePtr &root)
{
    json j = root;
    ofstream file(filename);
    file << j.dump(4); // pretty print with an indent of 4 spaces
    file.close();
}

void updateSceneGraph(const NodePtr &root)
{
    root->traverse([](Node *node)
                   { if (auto *cast = dynamic_cast<Transformable *>(node)) { cast->updateTransform(); } });
}
void renderSceneGraph(const NodePtr &root, ShaderObject *shader)
{
    root->traverse([&](Node *node)
                   { if (auto *cast = dynamic_cast<Renderable *>(node)) { cast->render(shader); } });
}
