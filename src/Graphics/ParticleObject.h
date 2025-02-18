#pragma once

#include <GLAD/glad.h>
#include <vector>
#include <string>
#include <memory>

#include <ResourceManager/ResourceManager.h>
#include <ResourceManager/MeshData.h>
#include <Graphics/ShaderObject.h>
#include <Graphics/TextureObject.h>
#include <Graphics/TextureArrayObject.h>

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
    float size;
    glm::vec4 color;
    float lifetime;
};

class ParticleObject
{
public:
    std::vector<Particle> particles;

    static constexpr float MAX_LIFETIME = 10.f;

    ParticleObject(const std::string &texture_path, const std::vector<Particle> &_particles);
    ~ParticleObject();

    void updateParticles();
    void updateInstanceBuffer();

    void render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation);

protected:
    GLuint quadVAO, quadVBO, instanceVBO;
    std::shared_ptr<TextureObject> texture;
    size_t deleted_particles;
    const float QuadVertices[12] = {
        -0.5f, -0.5f, 0.0f,   // bottom left
        0.5f, -0.5f, 0.0f,    // bottom right
        0.5f,  0.5f, 0.0f,    // top right
        -0.5f,  0.5f, 0.0f    // top left
    };

    void generateOpenGLBuffers();
    void populateOpenGLBuffers();
};