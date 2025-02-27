#include "BillboardNode.h"

void BillboardNode::render(const std::shared_ptr<ShaderObject> &_shader)
{
    if (!enabled || !visible)
        return;
    if (!texture)
    {
        texture = std::make_shared<TextureObject>(render_name);
    }
    if (!RenderObject::HasRenderObject("Billboard"))
    {
        auto data = std::make_shared<MeshData>();
        data->loadFromSource("v 0.000000 0.000000 0.000000\np 1", false);
        renderObject = RenderObject::AddRenderObject("Billboard", std::make_shared<RenderObject>(data));
    }
    if (!renderObject)
        renderObject = RenderObject::GetRenderObject("Billboard");
    auto shader = resolveShader(_shader);
    shader->use();
    shader->setInt("image", 0);
    shader->setInt("lockHorizontal", lockHorizontal ? 1 : 0);
    shader->setMat4("model", transform.globalTransform);
    texture->bind(0);
    renderObject->renderRaw();
}

void BillboardNode::reset()
{
    texture.reset();
    renderObject.reset();
}

void to_json(nlohmann::json &j, const std::shared_ptr<BillboardNode> &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const BillboardNode *node)
{
    ::to_json(j, dynamic_cast<const Transformable *>(node));
    j += {"billboardName", node->render_name};
    j += {"visible", node->visible};
    j += {"lockHorizontal", node->lockHorizontal};
}
void from_json(const nlohmann::json &j, const std::shared_ptr<BillboardNode> &node)
{
    ::from_json(j, std::dynamic_pointer_cast<Transformable>(node));
    j.at("billboardName").get_to(node->render_name);
    j.at("visible").get_to(node->visible);
    j.at("lockHorizontal").get_to(node->lockHorizontal);
}