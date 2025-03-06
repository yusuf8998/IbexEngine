#include <Graphics/ParticleObject.h>
#include <Graphics/GL.h>
#include <Graphics/Renderer.h>

ParticleObject::ParticleObject(const std::string &texture_path, const std::vector<Particle> &_particles)
    : deleted_particles(0)
{
    particles.insert(particles.begin(), _particles.begin(), _particles.end());

    generateOpenGLBuffers();
    populateOpenGLBuffers();

    texture = std::make_shared<TextureObject>(texture_path);
}

ParticleObject::~ParticleObject()
{
    glDeleteBuffers(1, &quadVBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteVertexArrays(1, &quadVAO);
}

void ParticleObject::updateParticles()
{
    auto dt = Renderer::instance().getDeltaTime();
    for (auto &p : particles)
    {
        if (p.lifetime == -1.f)
            continue;
        if (p.lifetime < MAX_LIFETIME)
        {
            p.lifetime += dt;
            continue;
        }
        p.lifetime = -1.f;
        deleted_particles++;
        p.acceleration = glm::vec3(0.f);
        p.velocity = glm::vec3(0.f);
        p.color = glm::vec4(glm::vec3(0.f), 1.f);
    }
}

void ParticleObject::updateInstanceBuffer()
{
    if (particles.size() == deleted_particles)
        return;
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Particle) * particles.size(), &particles[0], GL_DYNAMIC_DRAW);
}

void ParticleObject::generateOpenGLBuffers()
{
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &instanceVBO);
}

void ParticleObject::populateOpenGLBuffers()
{
    // Quad Stuff
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);

    // Position attribute (for the quad geometry)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Instance Stuff
    updateInstanceBuffer();

    // Set up the instance data (position, velocity, size, color, lifetime)
    size_t offset = 0;
    defineVertexAttrib(1, 3, sizeof(Particle), offset); // Position
    defineVertexAttrib(2, 3, sizeof(Particle), offset); // Velocity
    defineVertexAttrib(3, 3, sizeof(Particle), offset); // Acceleration
    defineVertexAttrib(4, 1, sizeof(Particle), offset); // Size
    defineVertexAttrib(5, 4, sizeof(Particle), offset); // Color
    defineVertexAttrib(6, 1, sizeof(Particle), offset); // Velocity

    // Set the instance data as "per-instance"
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
}

void ParticleObject::render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation)
{
    if (particles.size() == deleted_particles)
        return;
    // Check for OpenGL errors
    GLClearError();

    shader->use();
    shader->setMat4("model", transformation);
    Renderer::instance().slotTexture(GL_TEXTURE_2D, texture->getID(), shader, "image");

    // Bind the VAO, draw instances then unbind
    glBindVertexArray(quadVAO);
    GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, particles.size())); // For a quad, we render 4 vertices per instance
    glBindVertexArray(0);
}