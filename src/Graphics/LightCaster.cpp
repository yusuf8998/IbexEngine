#include <Graphics/LightCaster.h>
#include "LightCaster.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <Engine/Camera.h>

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

void DirectionalLight::calcLightSpaceMatrix()
{
    // glm::vec3 lightViewPos = -direction * 7.5f + mainCamera.position + mainCamera.front;
    // glm::vec3 lightViewCenter = mainCamera.position + mainCamera.front;

    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, .125f, 20.f);
    glm::mat4 lightView = glm::lookAt(-direction * 10.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
}
void PointLight::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".position", position);
    color.setUniforms(shader, name);
    attenuation.setUniforms(shader, name);
}

void PointLight::calcLightSpaceMatrix()
{
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.125f, 20.0f);
    glm::mat4 views[6];
    views[0] = glm::lookAt(position, position + glm::vec3(+1.0, +0.0, +0.0), glm::vec3(+0.0, -1.0, +0.0));
    views[1] = glm::lookAt(position, position + glm::vec3(-1.0, +0.0, +0.0), glm::vec3(+0.0, -1.0, +0.0));
    views[2] = glm::lookAt(position, position + glm::vec3(+0.0, +1.0, +0.0), glm::vec3(+0.0, +0.0, +1.0));
    views[3] = glm::lookAt(position, position + glm::vec3(+0.0, -1.0, +0.0), glm::vec3(+0.0, +0.0, -1.0));
    views[4] = glm::lookAt(position, position + glm::vec3(+0.0, +0.0, +1.0), glm::vec3(+0.0, -1.0, +0.0));
    views[5] = glm::lookAt(position, position + glm::vec3(+0.0, +0.0, -1.0), glm::vec3(+0.0, -1.0, +0.0));
    for (unsigned int i = 0; i < 6; i++)
        lightSpaceMatrix[i] = lightProjection * views[i];
}

void SpotLight::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".position", position);
    shader->setVec3(name + ".direction", direction);
    color.setUniforms(shader, name);
    attenuation.setUniforms(shader, name);
    cutOff.setUniforms(shader, name);
}

void SpotLight::calcLightSpaceMatrix()
{
    glm::mat4 lightProjection = glm::perspective(glm::radians(cutOff.outer * 2.0f), 1.0f, 0.125f, 20.f);
    glm::mat4 lightView = glm::lookAt(position, position + direction, glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
}