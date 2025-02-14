#include <Graphics/ParticleObject.h>
#include <Graphics/GLDebug.h>
#include <Graphics/Renderer.h>

ParticleObject::ParticleObject()
{
    generateOpenGLBuffers();
    populateOpenGLBuffers();
}

void ParticleObject::updateParticles()
{
    auto &renderer = Renderer::instance();
    for (auto &p : particles)
        p.lifetime += renderer.getDeltaTime();
}

void ParticleObject::updateInstanceBuffer()
{
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)0); // Position
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)(sizeof(glm::vec3))); // Velocity
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)(2 * sizeof(glm::vec3))); // Size
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)(2 * sizeof(glm::vec3) + sizeof(float))); // Color
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (void *)(2 * sizeof(glm::vec3) + sizeof(float) + sizeof(glm::vec4))); // Lifetime
    glEnableVertexAttribArray(5);

    // Set the instance data as "per-instance"
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
}

void ParticleObject::render(const std::shared_ptr<ShaderObject> &shader, const glm::mat4 &transformation)
{
    // Check for OpenGL errors
    GLClearError();

    shader->use();

    shader->setMat4("model", transformation);

    // Bind the VAO, draw instances then unbind
    glBindVertexArray(quadVAO);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, particles.size()); // For a quad, we render 4 vertices per instance
    glBindVertexArray(0);

    // Check for OpenGL errors
    ASSERT(GLLogCall("glDrawElements", __FILE__, __LINE__));
}