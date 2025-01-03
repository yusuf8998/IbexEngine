#pragma once

#include "SceneGraph.h"
#include <Graphics/MeshObject.h>

#include <Graphics/CubemapObject.h>

class SkyboxNode : public Renderable
{
private:
    std::shared_ptr<CubemapObject> cubeMap;
public:
    SkyboxNode(const std::string &name = "Unnamed")
        : Renderable(name) {}

    void render(ShaderObject *shader) override;
};
void to_json(nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node);
void to_json(nlohmann::json &j, const SkyboxNode *node);
void from_json(const nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node);