#include "SceneGraph.h"
#include <Graphics/RenderObject.h>
#include "SkyboxNode.h"
#include "BillboardNode.h"

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
    : name(name), parent(nullptr), children(), enabled(true)
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
        {"enabled", node->enabled},
        {"children", json::array()}};

    for (const auto &child : node->children)
    {
        j["children"].push_back(child);
    }

    if (auto *skybox = dynamic_cast<SkyboxNode *>(node.get()))
    {
        ::to_json(j, skybox);
    }
    else if (auto *billboard = dynamic_cast<BillboardNode *>(node.get()))
    {
        ::to_json(j, billboard);
    }
    else if (auto *renderable = dynamic_cast<Renderable *>(node.get()))
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
    j.at("enabled").get_to(node->enabled);
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
    if (j.contains("skyboxName"))
    {
        node = make_shared<SkyboxNode>();
        ::from_json(j, dynamic_pointer_cast<SkyboxNode>(node));
        return;
    }
    if (j.contains("lockHorizontal"))
    {
        node = make_shared<BillboardNode>();
        ::from_json(j, dynamic_pointer_cast<BillboardNode>(node));
        return;
    }
    if (j.contains("renderName"))
    {
        node = make_shared<Renderable>();
        ::from_json(j, dynamic_pointer_cast<Renderable>(node));
        return;
    }
    if (j.contains("activeChild"))
    {
        node = make_shared<SwitchNode>();
        ::from_json(j, dynamic_pointer_cast<SwitchNode>(node));
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

void to_json(nlohmann::json &j, const std::shared_ptr<SwitchNode> &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const SwitchNode *node)
{
    j += {"activeChild", node->active_child};
}
void from_json(const nlohmann::json &j, const std::shared_ptr<SwitchNode> &node)
{
    j.at("activeChild").get_to(node->active_child);
}

void Transformable::updateTransform(bool keep_global)
{
    if (is_static)
        transform.transformChanged = false;

    if (transform.transformChanged)
    {
        transform.applyTransformToLocal();
    }

    if (!keep_global)
    {
        transform.globalTransform = mat4(1.f);
        if (auto *parent_cast = dynamic_cast<Transformable *>(parent))
        {
            applyParentToGlobal(parent);
        }
        else if (auto *switch_cast = dynamic_cast<SwitchNode *>(parent))
        {
            applyParentToGlobal(parent->parent);
        }
        transform.applyLocalToGlobal();
    }

    transform.transformChanged = false;
}
void Transformable::applyParentToGlobal(Node *node)
{
    if (auto *parent_cast = dynamic_cast<Transformable *>(node))
    {
        transform.applyParentToGlobal(parent_cast->transform.globalTransform);
    }
}
void to_json(json &j, const TransformablePtr &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const Transformable *node)
{
    j += {"transform", node->transform};
    j += {"static", node->is_static};
}
void from_json(const json &j, const TransformablePtr &node)
{
    j.at("transform").get_to(node->transform);
    j.at("static").get_to(node->is_static);
    node->transform.transformChanged = true;
    node->updateTransform();
}

void Renderable::render(const std::shared_ptr<ShaderObject> &_shader)
{
    // Render the mesh
    auto shader = resolveShader(_shader);
    if (enabled && visible)
        RenderObject::GetRenderObject(render_name)->render(shader, transform.globalTransform);
}
void Renderable::reset()
{
    
}
const std::shared_ptr<ShaderObject> Renderable::resolveShader(const std::shared_ptr<ShaderObject> &shader) const
{
    if (forced_shader < 0)
        return shader;
    return Renderer::instance().getShader(forced_shader);
}
void to_json(json &j, const RenderablePtr &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const Renderable *node)
{
    ::to_json(j, dynamic_cast<const Transformable *>(node));
    j += {"renderName", node->render_name};
    j += {"visible", node->visible};
}
void from_json(const json &j, const RenderablePtr &node)
{
    ::from_json(j, dynamic_pointer_cast<Transformable>(node));
    j.at("renderName").get_to(node->render_name);
    j.at("visible").get_to(node->visible);
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
void renderSceneGraph(const NodePtr &root, const std::shared_ptr<ShaderObject> &shader)
{
    root->traverse([&](Node *node)
                   { if (auto *cast = dynamic_cast<Renderable *>(node)) { cast->render(shader); } });
}

void renderSceneGraph(const NodePtr &root, const std::shared_ptr<ShaderObject> &shader, bool ignore_forced_shaders)
{
    root->traverse([&](Node *node)
                   { if (auto *cast = dynamic_cast<Renderable *>(node)) { if (ignore_forced_shaders && cast->forced_shader != -1) return; cast->render(shader); } });
}