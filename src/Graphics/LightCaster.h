#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <Graphics/ShaderObject.h>
#include <stdexcept>

// float distance    = length(light.position - FragPos);
// float attenuation = 1.0 / (light.constant + light.linear * distance +
//     		    light.quadratic * (distance * distance));
// ambient  *= attenuation;
// diffuse  *= attenuation;
// specular *= attenuation;

// float theta = dot(lightDir, normalize(-light.direction));
// if(theta > light.cutOff)
// {
// //   do lighting calculations
// }
// else  // else, use ambient light so scene isn't completely dark outside the spotlight.
//   color = vec4(light.ambient * vec3(texture(material.diffuse, TexCoords)), 1.0);

struct LightColor
{
    glm::vec3 ambient, diffuse, specular;

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const;
};

struct LightAttenuation
{
    float constant, linear, quadratic;

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const;
};

struct LightCutOff
{
    float inner, outer;

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const;
};

struct LightCaster
{
    LightColor color;

    LightCaster(const LightColor &color)
        : color(color)
    {
    }
    LightCaster()
        : color(LightColor())
    {
    }
    virtual ~LightCaster() = 0;

    virtual void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const = 0;
    virtual void calcLightSpaceMatrix() = 0;
    virtual glm::mat4 getLightSpaceMatrix() const = 0;
};
inline LightCaster::~LightCaster() {}

struct AttenuationLightCaster : public LightCaster
{
    glm::vec3 position;
    LightAttenuation attenuation;

    AttenuationLightCaster(const LightColor &color, const LightAttenuation &attenuation)
        : LightCaster(color), attenuation(attenuation)
    {
    }
    AttenuationLightCaster()
        : LightCaster(), attenuation(LightAttenuation())
    {
    }
    virtual ~AttenuationLightCaster() = 0;
};
inline AttenuationLightCaster::~AttenuationLightCaster() {}

struct DirectionalLight : public LightCaster
{
    glm::vec3 direction, up;
    glm::mat4 lightSpaceMatrix;

    DirectionalLight(const LightColor &color)
        : LightCaster(color)
    {
    }
    DirectionalLight()
        : DirectionalLight(LightColor())
    {
    }

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const override;
    void calcLightSpaceMatrix() override;
    glm::mat4 getLightSpaceMatrix() const override { return lightSpaceMatrix; }
};

struct PointLight : public AttenuationLightCaster
{
    glm::mat4 lightSpaceMatrix[6];

    PointLight(const LightColor &color, const LightAttenuation &attenuation)
        : AttenuationLightCaster(color, attenuation)
    {
    }
    PointLight()
        : PointLight(LightColor(), LightAttenuation())
    {
    }

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const override;
    void calcLightSpaceMatrix() override;
    glm::mat4 getLightSpaceMatrix() const override { throw std::runtime_error("Can't get singular light space matrix of point light!"); }
};

struct SpotLight : public AttenuationLightCaster
{
    glm::vec3 direction, up;
    LightCutOff cutOff;

    glm::mat4 lightSpaceMatrix;

    SpotLight(const LightColor &color, const LightAttenuation &attenuation, const LightCutOff &cutOff)
        : AttenuationLightCaster(color, attenuation), cutOff(cutOff), direction(glm::vec3(0.f)), up(glm::vec3(0.f))
    {
    }
    SpotLight()
        : SpotLight(LightColor(), LightAttenuation(), LightCutOff())
    {
    }

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const override;
    void calcLightSpaceMatrix() override;
    glm::mat4 getLightSpaceMatrix() const override { return lightSpaceMatrix; }
};