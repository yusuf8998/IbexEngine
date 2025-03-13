#include "LightNode.h"

LightNode *LightNode::ActiveDirectionalLight;
std::array<LightNode *, MAX_POINT_LIGHTS> LightNode::ActivePointLights;
std::array<LightNode *, MAX_SPOT_LIGHTS> LightNode::ActiveSpotLights;

std::vector<LightNode *> LightNode::DirectionalLights;
std::vector<LightNode *> LightNode::PointLights;
std::vector<LightNode *> LightNode::SpotLights;

LightNode::LightNode(const std::string &name, std::vector<LightNode *> *vector)
    : Transformable(name)
{
    vector->push_back(this);
}

LightNode::LightNode(const std::string &name, const LightColor &color)
    : LightNode(name, &DirectionalLights)
{
    caster = std::make_shared<DirectionalLight>(color);
    shadowMap = std::make_shared<ShadowMap2D>();
}
LightNode::LightNode(const std::string &name, const LightColor &color, const LightAttenuation &attenuation)
    : LightNode(name, &PointLights)
{
    caster = std::make_shared<PointLight>(color, attenuation);
    shadowMap = std::make_shared<ShadowMapCube>();
}
LightNode::LightNode(const std::string &name, const LightColor &color, const LightAttenuation &attenuation, const LightCutOff &cutOff)
    : LightNode(name, &SpotLights)
{
    caster = std::make_shared<SpotLight>(color, attenuation, cutOff);
    shadowMap = std::make_shared<ShadowMap2D>();
}

LightNode::LightNode(const std::string &name)
    : Transformable(name)
{
}

LightNode::~LightNode()
{
    std::vector<LightNode *> *vector = 0;

    if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(caster))
        vector = &DirectionalLights;
    else if (auto point = std::dynamic_pointer_cast<PointLight>(caster))
        vector = &PointLights;
    else if (auto spot = std::dynamic_pointer_cast<SpotLight>(caster))
        vector = &SpotLights;
    else
    {
        std::cerr << "Unknown light type" << std::endl;
        return;
    }

    vector->erase(std::remove(vector->begin(), vector->end(), this), vector->end());
}

void LightNode::setCaster(const std::shared_ptr<LightCaster> &_caster)
{
    caster = _caster;

    std::vector<LightNode *> *vector = 0;
    if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(caster))
    {
        vector = &DirectionalLights;
        shadowMap = std::make_shared<ShadowMap2D>();
    }
    else if (auto point = std::dynamic_pointer_cast<PointLight>(caster))
    {
        vector = &PointLights;
        shadowMap = std::make_shared<ShadowMapCube>();
    }
    else if (auto spot = std::dynamic_pointer_cast<SpotLight>(caster))
    {
        vector = &SpotLights;
        shadowMap = std::make_shared<ShadowMap2D>();
    }

    vector->push_back(this);
}

void LightNode::updateVectors()
{
    if (auto point = std::dynamic_pointer_cast<PointLight>(caster))
    {
        point->position = transform.getGlobalPosition();
    }
    else if (auto spot = std::dynamic_pointer_cast<SpotLight>(caster))
    {
        spot->position = transform.getGlobalPosition();
        spot->direction = transform.getGlobalFront();
        spot->up = transform.getGlobalUp();
    }
    else if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(caster))
    {
        dir->direction = transform.getGlobalFront();
        dir->up = transform.getGlobalUp();
    }
    else
        throw std::runtime_error("Unknown light type");

    caster->calcLightSpaceMatrix();
}

void LightNode::renderDepth(const std::shared_ptr<ShaderObject> &shader, const std::function<void()> &renderFunc)
{
    if (!shadowMap)
        return;
    shadowMap->render(shader, caster, renderFunc);
}

int lastUsedShadowMapSlot = 0;

void LightNode::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    caster->setUniforms(shader, name);
    shadowMap->setUniforms(shader, name);
    lastUsedShadowMapSlot = shadowMap->getTextureSlot();
}

void LightNode::UpdateActiveLights()
{
    ActiveDirectionalLight = nullptr;
    ActivePointLights.fill(nullptr);
    ActiveSpotLights.fill(nullptr);

#ifdef DEBUG // Debug mode gives warning when too many lights are active, but takes more time updating (iterates all lights)
    for (auto dir : DirectionalLights)
    {
        if (!dir || !dir->isActive())
            continue;
        if (ActiveDirectionalLight != nullptr)
        {
            printf("Too many active directional lights\n");
            break;
        }
        ActiveDirectionalLight = dir;
    }

    int activePointLightCount = 0;
    for (auto point : PointLights)
    {
        if (!point || !point->isActive())
            continue;
        if (activePointLightCount >= MAX_POINT_LIGHTS)
        {
            printf("Too many active point lights\n");
            break;
        }
        ActivePointLights[activePointLightCount] = point;
        activePointLightCount++;
    }

    int activeSpotLightCount = 0;
    for (auto spot : SpotLights)
    {
        if (!spot || !spot->isActive())
            continue;
        if (activeSpotLightCount >= MAX_SPOT_LIGHTS)
        {
            printf("Too many active spot lights\n");
            break;
        }
        ActiveSpotLights[activeSpotLightCount] = spot;
        activeSpotLightCount++;
    }
#else // Release mode does not give warning when too many lights are active, but takes less time updating (stops when limit is met)
    for (auto &dir : DirectionalLights)
    {
        if (!dir || !dir->isActive())
            continue;
        ActiveDirectionalLight = dir;
        break;
    }

    int activePointLightCount = 0;
    for (auto point : PointLights)
    {
        if (!point || !point->isActive())
            continue;
        ActivePointLights[activePointLightCount] = point;
        activePointLightCount++;
        if (activePointLightCount >= MAX_POINT_LIGHTS)
            break;
    }

    int activeSpotLightCount = 0;
    for (auto spot : SpotLights)
    {
        if (!spot || !spot->isActive())
            continue;
        ActiveSpotLights[activeSpotLightCount] = spot;
        activeSpotLightCount++;
        if (activeSpotLightCount >= MAX_SPOT_LIGHTS)
            break;
    }
#endif
}

void LightNode::UpdateActiveLightVectors()
{
    if (ActiveDirectionalLight)
        ActiveDirectionalLight->updateVectors();
    for (auto point : ActivePointLights)
        if (point)
            point->updateVectors();
    for (auto spot : ActiveSpotLights)
        if (spot)
            spot->updateVectors();
}

void LightNode::SetActiveLightUniforms(const std::shared_ptr<ShaderObject> &shader)
{
    if (ActiveDirectionalLight)
    {
        ActiveDirectionalLight->setUniforms(shader, "dirLight");
        lightingUniforms.dirLight = *std::dynamic_pointer_cast<DirectionalLight>(ActiveDirectionalLight->caster).get();
    }
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        if (ActivePointLights[i])
        {
            ActivePointLights[i]->setUniforms(shader, "pointLights[" + std::to_string(i) + "]");
            lightingUniforms.pointLights[i] = *std::dynamic_pointer_cast<PointLight>(ActivePointLights[i]->caster).get();
        }
        else
            shader->setInt("pointLights[" + std::to_string(i) + "].shadowMap", lastUsedShadowMapSlot);
    lastUsedShadowMapSlot = 0;
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        if (ActiveSpotLights[i])
        {
            ActiveSpotLights[i]->setUniforms(shader, "spotLights[" + std::to_string(i) + "]");
            lightingUniforms.spotLights[i] = *std::dynamic_pointer_cast<SpotLight>(ActiveSpotLights[i]->caster).get();
        }
        else
            shader->setInt("spotLights[" + std::to_string(i) + "].shadowMap", lastUsedShadowMapSlot);
    lastUsedShadowMapSlot = 0;
}

void LightNode::RenderDepthMaps(const std::shared_ptr<ShaderObject> &shader, const std::function<void()> &renderFunc)
{
    glCullFace(GL_FRONT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (ActiveDirectionalLight)
        ActiveDirectionalLight->renderDepth(shader, renderFunc);
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        if (ActivePointLights[i])
            ActivePointLights[i]->renderDepth(shader, renderFunc);
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        if (ActiveSpotLights[i])
            ActiveSpotLights[i]->renderDepth(shader, renderFunc);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);
}

void to_json(nlohmann::json &j, const LightColor &color)
{
    j = nlohmann::json{
        {"ambient", color.ambient}, {"diffuse", color.diffuse}, {"specular", color.specular}};
}

void from_json(const nlohmann::json &j, LightColor &color)
{
    j.at("ambient").get_to(color.ambient);
    j.at("diffuse").get_to(color.diffuse);
    j.at("specular").get_to(color.specular);
}

void to_json(nlohmann::json &j, const LightAttenuation &attenuation)
{
    j = nlohmann::json{
        {"constant", attenuation.constant}, {"linear", attenuation.linear}, {"quadratic", attenuation.quadratic}};
}

void from_json(const nlohmann::json &j, LightAttenuation &attenuation)
{
    j.at("constant").get_to(attenuation.constant);
    j.at("linear").get_to(attenuation.linear);
    j.at("quadratic").get_to(attenuation.quadratic);
}

void to_json(nlohmann::json &j, const std::shared_ptr<LightNode> &node)
{
    ::to_json(j, node.get());
}
void to_json(nlohmann::json &j, const LightNode *node)
{
    ::to_json(j, dynamic_cast<const Transformable *>(node));
    if (auto point = std::dynamic_pointer_cast<PointLight>(node->getCaster()))
    {
        j += {"lightType", "point"};
        j += {"lightColor", point->color};
        j += {"lightAttenuation", point->attenuation};
    }
    else if (auto spot = std::dynamic_pointer_cast<SpotLight>(node->getCaster()))
    {
        j += {"lightType", "spot"};
        j += {"lightColor", spot->color};
        j += {"lightAttenuation", spot->attenuation};
        j += {"innerCutoff", spot->cutOff.inner};
        j += {"outerCutoff", spot->cutOff.outer};
    }
    else if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(node->getCaster()))
    {
        j += {"lightType", "directional"};
        j += {"lightColor", dir->color};
    }
    j += {"lightActive", node->isActive()};
}
void from_json(const nlohmann::json &j, const std::shared_ptr<LightNode> &node)
{
    ::from_json(j, std::dynamic_pointer_cast<Transformable>(node));
    std::string type;
    j.at("lightType").get_to(type);
    bool _active = false;
    j.at("lightActive").get_to(_active);
    node->setActive(_active);
    if (type == "point")
    {
        auto caster = std::make_shared<PointLight>();
        j.at("lightColor").get_to(caster->color);
        j.at("lightAttenuation").get_to(caster->attenuation);
        node->setCaster(caster);
    }
    else if (type == "spot")
    {
        auto caster = std::make_shared<SpotLight>();
        j.at("lightColor").get_to(caster->color);
        j.at("lightAttenuation").get_to(caster->attenuation);
        j.at("innerCutoff").get_to(caster->cutOff.inner);
        j.at("outerCutoff").get_to(caster->cutOff.outer);
        node->setCaster(caster);
    }
    else if (type == "directional")
    {
        auto caster = std::make_shared<DirectionalLight>();
        j.at("lightColor").get_to(caster->color);
        node->setCaster(caster);
    }
    else
        throw std::runtime_error("Unknown light type: " + type);
}