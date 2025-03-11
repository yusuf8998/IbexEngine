#include <Graphics/ShadowMap.h>
#include <glm/gtc/matrix_transform.hpp>
#include "ShadowMap.h"
#include "Renderer.h"

ShadowMap::ShadowMap(unsigned int width, unsigned int height)
    : width(width), height(height)
{
}

ShadowMap::~ShadowMap()
{
    glDeleteTextures(1, &id);
}

void ShadowMap::bind() const
{
    fbo->bind();
    glBindTexture(target, id);
}

void ShadowMap::unbind() const
{
    fbo->unbind();
    glBindTexture(target, 0);
}

void ShadowMap::prepare(const std::shared_ptr<ShaderObject> &shader, glm::mat4 lightSpaceMatrix)
{
    fbo->setTexture(id, target, GL_DEPTH_ATTACHMENT);
    shader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
}

void ShadowMap::setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name)
{
    // Renderer::instance().slotTexture(target, id, shader, name + ".shadowMap");
    textureSlot = Renderer::instance().slotTexture(target, id);
    shader->setInt(name + ".shadowMap", textureSlot);
    // shader->setFloat(name + ".farPlane", 7.5f);
    // shader->setFloat(name + ".nearPlane", 1.0f);
    // shader->setFloat(name + ".shadowBias", 0.005f);
    // shader->setFloat(name + ".shadowStrength", 0.5f);
    // shader->setFloat(name + ".shadowMapWidth", static_cast<float>(width));
    // shader->setFloat(name + ".shadowMapHeight", static_cast<float>(height));
}

FramebufferObject *ShadowMap2D::FBO = 0;

void ShadowMap2D::generateTexture()
{
    target = GL_TEXTURE_2D;
    glGenTextures(1, &id);
    glBindTexture(target, id);
    glTexImage2D(target, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(target, 0);
}

ShadowMap2D::ShadowMap2D()
    : ShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT)
{
    if (!FBO)
    {
        FBO = new FramebufferObject(width, height, true, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    }
    fbo = FBO;
    generateTexture();
}

void ShadowMap2D::render(const std::shared_ptr<ShaderObject> &shader, const std::shared_ptr<LightCaster> &light, const std::function<void()> &renderFunc)
{
    bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    prepare(shader, light->getLightSpaceMatrix());
    renderFunc();
    unbind();
}

FramebufferObject *ShadowMapCube::FBO = 0;

void ShadowMapCube::generateTexture()
{
    target = GL_TEXTURE_CUBE_MAP;
    glGenTextures(1, &id);
    glBindTexture(target, id);
    for (unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(target, 0);
}

ShadowMapCube::ShadowMapCube()
    : ShadowMap(SHADOW_WIDTH, SHADOW_HEIGHT)
{
    generateTexture();
    if (!FBO)
    {
        FBO = new FramebufferObject(width, height, true, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    }
    fbo = FBO;
}

void ShadowMapCube::render(const std::shared_ptr<ShaderObject> &shader, const std::shared_ptr<LightCaster> &light, const std::function<void()> &renderFunc)
{
    bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    auto point = std::dynamic_pointer_cast<PointLight>(light);
    for (target = GL_TEXTURE_CUBE_MAP_POSITIVE_X; target < GL_TEXTURE_CUBE_MAP_POSITIVE_X + 6; target++)
    {
        prepare(shader, point->lightSpaceMatrix[target - GL_TEXTURE_CUBE_MAP_POSITIVE_X]);
        renderFunc();
    }
    target = GL_TEXTURE_CUBE_MAP;
    unbind();
}