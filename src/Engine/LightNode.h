#pragma once

#include <Graphics/LightCaster.h>
#include <Engine/SceneGraph.h>

constexpr int MAX_POINT_LIGHTS = 4;
constexpr int MAX_SPOT_LIGHTS = 4;

class LightNode : public Transformable, public std::enable_shared_from_this<LightNode>
{
protected:
    std::shared_ptr<LightCaster> caster = nullptr;
    bool active = false;

    static LightNode * ActiveDirectionalLight;
    static std::array<LightNode *, MAX_POINT_LIGHTS> ActivePointLights;
    static std::array<LightNode *, MAX_SPOT_LIGHTS> ActiveSpotLights;

    static std::vector<LightNode *> DirectionalLights;
    static std::vector<LightNode *> PointLights;
    static std::vector<LightNode *> SpotLights;

    LightNode(const std::string &name, std::vector<LightNode *> *vector);

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const;

public:
    // Directional
    LightNode(const std::string &name, const LightColor &color);
    // Point
    LightNode(const std::string &name, const LightColor &color, const LightAttenuation &attenuation);
    // Spot
    LightNode(const std::string &name, const LightColor &color, const LightAttenuation &attenuation, const LightCutOff &cutOff);
    // Null Caster
    LightNode(const std::string &name = "Unnamed");

    ~LightNode();

    std::shared_ptr<LightCaster> getCaster() { return caster; }
    const std::shared_ptr<LightCaster> getCaster() const { return caster; }
    bool isActive() const { return active; }
    void setActive(bool _active) { active = _active; }
    void setCaster(const std::shared_ptr<LightCaster> &_caster);

    void updateVectors();

    static void UpdateActiveLights();
    static void UpdateActiveLightVectors();
    static void SetActiveLightUniforms(const std::shared_ptr<ShaderObject> &shader);
};

void to_json(nlohmann::json &j, const LightColor &color);
void from_json(const nlohmann::json &j, LightColor &color);

void to_json(nlohmann::json &j, const LightAttenuation &attenuation);
void from_json(const nlohmann::json &j, LightAttenuation &attenuation);

void to_json(nlohmann::json &j, const std::shared_ptr<LightNode> &node);
void to_json(nlohmann::json &j, const LightNode *node);
void from_json(const nlohmann::json &j, const std::shared_ptr<LightNode> &node);