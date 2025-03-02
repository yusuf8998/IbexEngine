#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <Graphics/ShaderObject.h>

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
    virtual ~LightCaster() = 0;

    virtual void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const = 0;
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
    virtual ~AttenuationLightCaster() = 0;
};
inline AttenuationLightCaster::~AttenuationLightCaster() {}

struct DirectionalLight : public LightCaster
{
    glm::vec3 direction;

    DirectionalLight(const LightColor &color)
        : LightCaster(color)
    {
    }

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const override;
};

struct PointLight : public AttenuationLightCaster
{
    PointLight(const LightColor &color, const LightAttenuation &attenuation)
        : AttenuationLightCaster(color, attenuation)
    {
    }

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const override;
};

struct SpotLight : public AttenuationLightCaster
{
    glm::vec3 direction;
    LightCutOff cutOff;

    SpotLight(const LightColor &color, const LightAttenuation &attenuation, const LightCutOff &cutOff)
        : AttenuationLightCaster(color, attenuation), cutOff(cutOff)
    {
    }

    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const override;
};