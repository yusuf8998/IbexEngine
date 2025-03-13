#include <Graphics/LightCaster.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <Engine/Camera.h>
#include "LightCaster.h"

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
    shader->setMat4(name + ".lightSpaceMatrix", lightSpaceMatrix);
}

void DirectionalLight::calcLightSpaceMatrix()
{
    // glm::vec3 lightViewPos = -direction * 7.5f + mainCamera.position + mainCamera.front;
    // glm::vec3 lightViewCenter = mainCamera.position + mainCamera.front;

    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.f, 7.5f);
    glm::mat4 lightView = glm::lookAt(-direction * 1.0f, glm::vec3(0.0f), up);
    lightSpaceMatrix = lightProjection * lightView;
}
void PointLight::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const
{
    shader->setVec3(name + ".position", position);
    color.setUniforms(shader, name);
    attenuation.setUniforms(shader, name);
    for (int i = 0; i < 6; i++)
        shader->setMat4(name + ".lightSpaceMatrix[" + std::to_string(i) + "]", lightSpaceMatrix[i]);
}

void PointLight::calcLightSpaceMatrix()
{
    glm::mat4 lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.125f, 20.0f);
    glm::mat4 views[6];
    views[0] = glm::lookAt(position, position + glm::vec3(+1.0, +0.0, +0.0), glm::vec3(+0.0, +1.0, +0.0));
    views[1] = glm::lookAt(position, position + glm::vec3(-1.0, +0.0, +0.0), glm::vec3(+0.0, +1.0, +0.0));
    views[2] = glm::lookAt(position, position + glm::vec3(+0.0, +1.0, +0.0), glm::vec3(+0.0, +0.0, +1.0));
    views[3] = glm::lookAt(position, position + glm::vec3(+0.0, -1.0, +0.0), glm::vec3(+0.0, +0.0, -1.0));
    views[4] = glm::lookAt(position, position + glm::vec3(+0.0, +0.0, +1.0), glm::vec3(+0.0, +1.0, +0.0));
    views[5] = glm::lookAt(position, position + glm::vec3(+0.0, +0.0, -1.0), glm::vec3(+0.0, +1.0, +0.0));
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
    shader->setMat4(name + ".lightSpaceMatrix", lightSpaceMatrix);
}

void SpotLight::calcLightSpaceMatrix()
{
    glm::mat4 lightProjection = glm::perspective(glm::radians(cutOff.outer * 2.0f), 1.0f, 0.125f, 20.f);
    glm::mat4 lightView = glm::lookAt(position, position + direction, up);
    lightSpaceMatrix = lightProjection * lightView;
}

#include <Graphics/UniformBufferObject.h>
void pushLightingColorData(UniformBufferObject &ubo, const LightColor &color, size_t &offset)
{
    ubo.pushData(&color.ambient[0], sizeof(glm::vec3), offset);
    ubo.pushData(&color.diffuse[0], sizeof(glm::vec3), offset);
    ubo.pushData(&color.specular[0], sizeof(glm::vec3), offset);
}
void pushLightingAttenuationData(UniformBufferObject &ubo, const LightAttenuation &attenuation, size_t &offset)
{
    glm::vec3 attenuation_vector = {attenuation.constant, attenuation.linear, attenuation.quadratic};
    ubo.pushData(&attenuation_vector[0], sizeof(glm::vec3), offset);
}
void pushLightingCutoffData(UniformBufferObject &ubo, const LightCutOff &cutoff, size_t &offset)
{
    glm::vec2 cutoff_vector = {cutoff.inner, cutoff.outer};
    ubo.pushData(&cutoff_vector[0], sizeof(glm::vec2), offset);
}
void setLightingData(UniformBufferObject &ubo, const LightingUniforms &uniforms)
{
    size_t offset = 0;

    ubo.pushData(&uniforms.viewPos[0], sizeof(glm::vec3), offset);

    // Directional Light
    ubo.pushData(&uniforms.dirLight.direction[0], sizeof(glm::vec3), offset);
    ubo.pushData(&uniforms.dirLight.up[0], sizeof(glm::vec3), offset);
    pushLightingColorData(ubo, uniforms.dirLight.color, offset);
    ubo.pushData(&uniforms.dirLight.lightSpaceMatrix[0][0], sizeof(glm::mat4), offset);

    // Point Lights
    for (int i = 0; i < MAX_POINT_LIGHTS; i++)
    {
        ubo.pushData(&uniforms.pointLights[i].position[0], sizeof(glm::vec3), offset);
        pushLightingColorData(ubo, uniforms.pointLights[i].color, offset);
        pushLightingAttenuationData(ubo, uniforms.pointLights[i].attenuation, offset);
        for (int j = 0; j < 6; j++)
            ubo.pushData(&uniforms.pointLights[i].lightSpaceMatrix[j][0][0], sizeof(glm::mat4), offset);
    }

    // Spot Lights
    for (int i = 0; i < MAX_SPOT_LIGHTS; i++)
    {
        ubo.pushData(&uniforms.spotLights[i].position[0], sizeof(glm::vec3), offset);
        ubo.pushData(&uniforms.spotLights[i].direction[0], sizeof(glm::vec3), offset);
        ubo.pushData(&uniforms.spotLights[i].up[0], sizeof(glm::vec3), offset);
        pushLightingColorData(ubo, uniforms.spotLights[i].color, offset);
        pushLightingAttenuationData(ubo, uniforms.spotLights[i].attenuation, offset);
        pushLightingCutoffData(ubo, uniforms.spotLights[i].cutOff, offset);
        ubo.pushData(&uniforms.spotLights[i].lightSpaceMatrix[0][0], sizeof(glm::mat4), offset);
    }
}