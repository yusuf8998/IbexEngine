#include "SkyboxNode.h"
#include "Graphics/Renderer.h"

void to_json(nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const SkyboxNode *node)
{
    j += {"skyboxName", node->meshName};
    j += {"visible", node->visible};
}
void from_json(const nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node)
{
    j.at("skyboxName").get_to(node->meshName);
    j.at("visible").get_to(node->visible);
    node->static_ = true;
    node->getTransform().position = glm::vec3(0.f);
    node->getTransform().rotation = glm::quat(glm::vec3(0.f));
    node->getTransform().scale = glm::vec3(0.f);
    // MeshObject::GetMeshObject(node->meshName)->data->
}

void SkyboxNode::setCubemap(const std::array<std::string, 6> &sides)
{
    cubeMap = std::make_shared<CubemapObject>(sides);
}

void SkyboxNode::setCubemap(const std::string &cubemapDir, const std::string &extension)
{
    cubeMap = std::make_shared<CubemapObject>(cubemapDir, extension);
}

void SkyboxNode::render(const std::shared_ptr<ShaderObject> &_shader)
{
    if (!enabled || !visible)
        return;
    cubeMap->bind(GL_TEXTURE0);
    auto shader = Renderer::instance().getSkyboxShader();
    shader->use();
    shader->setInt("cubemap", 0);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    MeshObject::GetMeshObject(meshName)->renderRaw();
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}