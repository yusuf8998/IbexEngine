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
    int illum;

    // Texture maps (can be extended to support more textures)
    std::string diffuseTexture; // Filepath for diffuse texture
    std::string specularTexture; // Filepath for specular texture
    std::string normalMap; // Filepath for normal map
    std::string dispMap; // Filepath for displacement map

    Material(const glm::vec3 &diff = glm::vec3(0.0f), const glm::vec3 &spec = glm::vec3(0.0f), const glm::vec3 &amb = glm::vec3(0.0f), float shin = 32.0f)
        : diffuse(diff), specular(spec), ambient(amb), shininess(shin), illum(-1), diffuseTexture(""), specularTexture(""), normalMap(""), dispMap("") {}

    inline std::vector<std::string> getTextures() const
    {
        std::vector<std::string> result = {};
        if (!diffuseTexture.empty())
            result.push_back(diffuseTexture);
        if (!specularTexture.empty())
            result.push_back(specularTexture);
        if (!normalMap.empty())
            result.push_back(normalMap);
        if (!dispMap.empty())
            result.push_back(dispMap);
        return result;
    }
};