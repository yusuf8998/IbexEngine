#pragma once

#include <GLAD/glad.h>
#include <stddef.h>
#include <string>

class UniformBufferObject
{
protected:
    GLuint ubo;
    GLuint bindingPoint;
    size_t size;
    std::string name;
    char *data;

public:
    UniformBufferObject(const std::string &name, size_t size);
    ~UniformBufferObject();

    void bind(GLuint program);
    void unbind() const;
    void setData(const void *data) const;
    void setSubData(const void *data, size_t offset, size_t length) const;
    GLuint getBindingPoint() const;
    size_t getSize() const;
    const std::string &getName() const;
    char *getData() const;
};

#include <glm/glm.hpp>
namespace ShaderUniforms
{
    constexpr int MAX_POINT_LIGHTS = 4;
    constexpr int MAX_SPOT_LIGHTS = 4;

    struct LightColor {
        glm::vec3 ambient, diffuse, specular;
    };
    // 3 x 3 = 9 locations
    
    struct LightAttenuation {
        float constant, linear, quadratic;
    };
    // 3 locations
    
    struct LightCutOff {
        float inner, outer;
    };
    // 2 locations
    
    struct DirectionalLight {
        glm::vec3 direction; // 3 locations
        LightColor color; // 9 locations
        int shadowMap; // 1 locations
        glm::mat4 lightSpaceMatrix; // 4 x 4 = 16 locations
    };
    // 3 + 9 + 1 + 16 = 29 locations

    struct PointLight {
        glm::vec3 position; // 3 locations
        LightColor color; // 9 locations
        LightAttenuation attenuation; // 3 locations
        int shadowMap; // 1 locations
        glm::mat4 lightSpaceMatrix[6]; // 6 x 4 x 4 = 96 locations
    };
    // 3 + 9 + 3 + 1 + 96 = 112 locations
    
    struct SpotLight {
        glm::vec3 position; // 3 locations
        glm::vec3 direction; // 3 locations
        LightColor color; // 9 locations
        LightAttenuation attenuation; // 3 locations
        LightCutOff cutOff; // 2 locations
        int shadowMap; // 1 locations
        glm::mat4 lightSpaceMatrix; // 4 x 4 = 16 locations
    };
    // 3 + 3 + 9 + 3 + 2 + 1 + 16 = 37 locations

    struct LightingUniforms
    {
        DirectionalLight dirLight; // 29 locations
        PointLight pointLights[MAX_POINT_LIGHTS]; // 112 x 4 = 448 locations
        SpotLight SpotLights[MAX_SPOT_LIGHTS]; // 37 x 4 = 148 locations
        glm::vec3 viewPos; // 3 locations
    };
    // 29 + 448 + 148 + 3 = 628 locations
    constexpr size_t LIGHTING_UNIFORM_SIZE = sizeof(LightingUniforms);
    
} // namespace ShaderUniforms
