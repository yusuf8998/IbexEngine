#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <vector>

// A simple Material class holding basic material properties
class Material
{
public:
    // Properties for the material (diffuse, specular, ambient colors)
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float shininess;

    // Texture maps (can be extended to support more textures)
    std::string diffuseTexture; // Filepath for diffuse texture

    Material(const glm::vec3 &diff = glm::vec3(0.0f), const glm::vec3 &spec = glm::vec3(0.0f), const glm::vec3 &amb = glm::vec3(0.0f), float shin = 32.0f)
        : diffuse(diff), specular(spec), ambient(amb), shininess(shin) {}
};