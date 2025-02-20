#pragma once

#include "SceneGraph.h"
#include <Graphics/RenderObject.h>
#include <Graphics/Renderer.h>
#include <Graphics/CubemapObject.h>

class SkyboxNode : public Renderable
{
private:
    std::shared_ptr<CubemapObject> cubeMap;
    const std::string SkyboxMesh = "res/Models/skybox.obj";

public:
    SkyboxNode(const std::string &name = "Unnamed")
        : Renderable(name) { forced_shader = Renderer::instance().getSkyboxShaderIndex(); }

    void setCubemap(const std::array<std::string, 6> &sides);
    void setCubemap(const std::string &cubemapDir, const std::string &extension = "png");
    void render(const std::shared_ptr<ShaderObject> &shader) override;
    void reset() override;
};
void to_json(nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node);
void to_json(nlohmann::json &j, const SkyboxNode *node);
void from_json(const nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node);