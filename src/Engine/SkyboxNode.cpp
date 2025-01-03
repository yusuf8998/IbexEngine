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

void SkyboxNode::render(ShaderObject *shader)
{
    if (!enabled || !visible)
        return;
    
    MeshObject::GetMeshObject(meshName)->render(Renderer::instance().getSkyboxShader().get(), glm::mat4(1.f));
}