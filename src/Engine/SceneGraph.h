#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <iostream>

#include "Transform.h"
#include <Graphics/ShaderObject.h>

typedef std::shared_ptr<class Node> NodePtr;
typedef std::shared_ptr<class Transformable> TransformablePtr;
typedef std::shared_ptr<class Renderable> RenderablePtr;

class Node
{
protected:
    mutable std::shared_mutex mutex_;

public:
    std::string name;
    Node *parent;
    std::vector<NodePtr> children;
    bool enabled;

    Node(const std::string &name = "Unnamed");
    virtual ~Node() = default;

    virtual inline void traverse(std::function<void(Node *)> f)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        f(this);
        for (auto &child : children)
        {
            child->traverse(f);
        }
    }

    void addChild(NodePtr child);
};
void to_json(nlohmann::json &j, const NodePtr &node);
void from_json(const nlohmann::json &j, NodePtr &node);

void constructNodeFromJson(const nlohmann::json &j, NodePtr &node);

class SwitchNode : public Node
{
public:
    int active_child;

    SwitchNode(const std::string &name = "Unnamed")
        : Node(name) {}
    
    inline void traverse(std::function<void(Node *)> f) override
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        f(this);
        children[active_child]->traverse(f);
    }
};
void to_json(nlohmann::json &j, const std::shared_ptr<SwitchNode> &node);
void to_json(nlohmann::json &j, const SwitchNode *node);
void from_json(const nlohmann::json &j, const std::shared_ptr<SwitchNode> &node);

class Transformable : public Node
{
public:
    Transform transform;
    bool is_static;

    Transformable(const std::string &name = "Unnamed")
        : Node(name), transform(), is_static(false) {}

    void updateTransform(bool keep_global = false);

protected:

    void applyParentToGlobal(Node *node);
};

void to_json(nlohmann::json &j, const TransformablePtr &node);
void to_json(nlohmann::json &j, const Transformable *node);
void from_json(const nlohmann::json &j, const TransformablePtr &node);

class Renderable : public Transformable
{
public:
    std::string render_name;
    bool visible;
    int forced_shader;

    Renderable(const std::string &name = "Unnamed")
        : Transformable(name), render_name(""), visible(true), forced_shader(-1) {}

    virtual void render(const std::shared_ptr<ShaderObject> &shader);
    virtual void reset();

protected:
    const std::shared_ptr<ShaderObject> resolveShader(const std::shared_ptr<ShaderObject> &shader) const;
};
void to_json(nlohmann::json &j, const RenderablePtr &node);
void to_json(nlohmann::json &j, const Renderable *node);
void from_json(const nlohmann::json &j, const RenderablePtr &node);

void loadSceneGraph(const std::string &filename, NodePtr &root);
void saveSceneGraph(const std::string &filename, const NodePtr &root);

template <typename T>
inline std::shared_ptr<T> makeNode(const std::string &name = "Unnamed")
{
    return std::make_shared<T>(name);
}

template <typename T>
inline std::shared_ptr<T> castNode(const NodePtr &node)
{
    return std::dynamic_pointer_cast<T>(node);
}

void updateSceneGraph(const NodePtr &root);
void renderSceneGraph(const NodePtr &root, const std::shared_ptr<ShaderObject> &shader);
void renderSceneGraph(const NodePtr &root, const std::shared_ptr<ShaderObject> &shader, bool ignore_forced_shaders);