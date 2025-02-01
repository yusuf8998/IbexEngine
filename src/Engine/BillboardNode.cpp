#include "BillboardNode.h"

void BillboardNode::render(const std::shared_ptr<ShaderObject> &_shader)
{
    if (!enabled || !visible)
        return;
    if (!texture)
    {
        texture = std::make_shared<TextureObject>(renderName);
    }
    if (!renderObject)
    {
        auto data = std::make_shared<MeshData>();
        data->loadFromSource("v 0.000000 0.000000 0.000000\np 1", false);
        renderObject = RenderObject::AddRenderObject(name, std::make_shared<RenderObject>(data));
    }
    auto shader = resolveShader(_shader);
    shader->use();
    shader->setInt("image", 0);
    shader->setMat4("model", transform.globalTransform);
    texture->bind(GL_TEXTURE0);
    RenderObject::GetRenderObject(name)->renderRaw();
}