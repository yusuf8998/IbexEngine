#include "LightNode.h"

LightNode * LightNode::ActiveDirectionalLight;
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
}
LightNode::LightNode(const std::string &name, const LightColor &color, const LightAttenuation &attenuation)
    : LightNode(name, &PointLights)
{
    caster = std::make_shared<PointLight>(color, attenuation);
}
LightNode::LightNode(const std::string &name, const LightColor &color, const LightAttenuation &attenuation, const LightCutOff &cutOff)
    : LightNode(name, &SpotLights)
{
    caster = std::make_shared<SpotLight>(color, attenuation, cutOff);
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
    // else
    //     throw std::runtime_error("Unknown light type");

    vector->erase(std::remove(vector->begin(), vector->end(), this), vector->end());
}

void LightNode::setCaster(const std::shared_ptr<LightCaster> &_caster)
{
    caster = _caster;

    std::vector<LightNode *> *vector = 0;
    if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(caster))
        vector = &DirectionalLights;
    else if (auto point = std::dynamic_pointer_cast<PointLight>(caster))
        vector = &PointLights;
    else if (auto spot = std::dynamic_pointer_cast<SpotLight>(caster))
        vector = &SpotLights;

    vector->push_back(this);
}

void LightNode::updateVectors()
{
    if (auto point = std::dynamic_pointer_cast<PointLight>(caster))
    {
        point->position = transform.getPosition();
    }
    else if (auto spot = std::dynamic_pointer_cast<SpotLight>(caster))
    {
        spot->position = transform.getPosition();
        spot->direction = transform.getFront();
    }
    else if (auto dir = std::dynamic_pointer_cast<DirectionalLight>(caster))
    {
        dir->direction = transform.getFront();
    }
    else
        throw std::runtime_error("Unknown light type");
}

void LightNode::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    caster->setUniforms(shader, name);
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
        ActiveDirectionalLight->setUniforms(shader, "dirLight");
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
        if (ActivePointLights[i])
            ActivePointLights[i]->setUniforms(shader, "pointLights[" + std::to_string(i) + "]");
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
        if (ActiveSpotLights[i])
            ActiveSpotLights[i]->setUniforms(shader, "spotLights[" + std::to_string(i) + "]");
}