#pragma once

#include <Engine/SceneGraph.h>
#include <Graphics/TextureObject.h>
#include <ResourceManager/MeshData.h>
#include <Graphics/RenderObject.h>

class BillboardNode : public Renderable
{
public:
    BillboardNode(const std::string &name = "Unnamed")
        : Renderable(name) { forced_shader = 3; }

    void render(const std::shared_ptr<ShaderObject> &shader) override;

private:
    std::shared_ptr<TextureObject> texture;
    std::shared_ptr<RenderObject> renderObject;
};
void to_json(nlohmann::json &j, const std::shared_ptr<BillboardNode> &node);
void to_json(nlohmann::json &j, const BillboardNode *node);
void from_json(const nlohmann::json &j, const std::shared_ptr<BillboardNode> &node);