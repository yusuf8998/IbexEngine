#include "SkyboxNode.h"

void to_json(nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const SkyboxNode *node)
{
    j += {"skyboxName", node->render_name};
    j += {"visible", node->visible};
}
void from_json(const nlohmann::json &j, const std::shared_ptr<SkyboxNode> &node)
{
    j.at("skyboxName").get_to(node->render_name);
    j.at("visible").get_to(node->visible);
    node->is_static = true;
    node->transform.position = glm::vec3(0.f);
    node->transform.rotation = glm::quat(glm::vec3(0.f));
    node->transform.scale = glm::vec3(0.f);
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
    if (!cubeMap)
    {
        auto split = splitString(render_name, '*');
        assert(split.size() == 2);
        setCubemap(split[0], split[1]);
    }
    if (!SkyboxObject)
        SkyboxObject = RenderObject::GetRenderObject(SkyboxMesh);
    cubeMap->bind(0);
    auto shader = resolveShader(_shader);
    shader->use();
    shader->setInt("cubemap", 0);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    SkyboxObject->renderRaw();
    glDepthFunc(DEFAULT_DEPTH_FUNC);
    glDepthMask(GL_TRUE);
}

void SkyboxNode::reset()
{
    cubeMap.reset();
}