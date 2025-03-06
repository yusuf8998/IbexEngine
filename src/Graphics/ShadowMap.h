#pragma once

#include <Graphics/FramebufferObject.h>
#include <Graphics/ShaderObject.h>
#include <Graphics/LightCaster.h>

#include <functional>

class ShadowMap
{
protected:
    GLuint id, target;
    FramebufferObject *fbo;
    unsigned int width, height;

    virtual void generateTexture() = 0;

public:
    ShadowMap(unsigned int width, unsigned int height);
    ~ShadowMap();

    void bind() const;
    void unbind() const;

    GLuint getID() const { return id; }
    const FramebufferObject *getFBO() const { return fbo; }

    unsigned int getWidth() const { return width; }
    unsigned int getHeight() const { return height; }

    void prepare(const std::shared_ptr<ShaderObject> &shader, glm::mat4 lightSpaceMatrix);
    void setUniforms(const std::shared_ptr<ShaderObject> &shader, const std::string &name) const;

    virtual void render(const std::shared_ptr<ShaderObject> &shader, const std::shared_ptr<LightCaster> &light, const std::function<void()> &renderFunc) = 0;
};

class ShadowMap2D : public ShadowMap
{
protected:
    static FramebufferObject *FBO;
    void generateTexture() override;

public:
    static const unsigned int SHADOW_WIDTH = 1024;
    static const unsigned int SHADOW_HEIGHT = 1024;

    ShadowMap2D();
    ~ShadowMap2D() = default;

    void render(const std::shared_ptr<ShaderObject> &shader, const std::shared_ptr<LightCaster> &light, const std::function<void()> &renderFunc) override;
};

class ShadowMapCube : public ShadowMap
{
protected:
    static FramebufferObject *FBO;
    void generateTexture() override;

public:
    static const unsigned int SHADOW_WIDTH = 1024;
    static const unsigned int SHADOW_HEIGHT = 1024;

    ShadowMapCube();
    ~ShadowMapCube() = default;

    void render(const std::shared_ptr<ShaderObject> &shader, const std::shared_ptr<LightCaster> &light, const std::function<void()> &renderFunc) override;
};