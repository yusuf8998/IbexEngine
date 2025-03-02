#include <Graphics/LightCaster.h>

void LightColor::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".color.ambient", ambient);
    shader->setVec3(name + ".color.diffuse", diffuse);
    shader->setVec3(name + ".color.specular", specular);
}

void LightAttenuation::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setFloat(name + ".attenuation.constant", constant);
    shader->setFloat(name + ".attenuation.linear", linear);
    shader->setFloat(name + ".attenuation.quadratic", quadratic);
}

void LightCutOff::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setFloat(name + ".cutOff.inner", inner);
    shader->setFloat(name + ".cutOff.outer", outer);
}

void DirectionalLight::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".direction", direction);
    color.setUniforms(shader, name);
}

void PointLight::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".position", position);
    color.setUniforms(shader, name);
    attenuation.setUniforms(shader, name);
}

void SpotLight::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".position", position);
    shader->setVec3(name + ".direction", direction);
    color.setUniforms(shader, name);
    attenuation.setUniforms(shader, name);
    cutOff.setUniforms(shader, name);
}