#pragma once

#include <Engine/SceneGraph.h>
#include <Graphics/TextureObject.h>
#include <ResourceManager/MeshData.h>
#include <Graphics/RenderObject.h>

class BillboardNode : public Renderable
{
public:
    bool lockHorizontal;

    BillboardNode(const std::string &name = "Unnamed")
        : Renderable(name), lockHorizontal(false) { forced_shader = 3; }

    void render(const std::shared_ptr<ShaderObject> &shader) override;
    void reset() override;

private:
    std::shared_ptr<TextureObject> texture;
};
void to_json(nlohmann::json &j, const std::shared_ptr<BillboardNode> &node);
void to_json(nlohmann::json &j, const BillboardNode *node);
void from_json(const nlohmann::json &j, const std::shared_ptr<BillboardNode> &node);