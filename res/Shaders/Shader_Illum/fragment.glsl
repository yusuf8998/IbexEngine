#version 450

struct Material {
    sampler2DArray textures;
    int diffuseIndex;
    int specularIndex;
    int normalIndex;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

uniform vec3 viewPos;

layout(location = 0) in vec3 f_fragPos;
layout(location = 1) in vec2 f_texCoords;
layout(location = 2) in vec3 f_tangentLightDir;
layout(location = 3) in vec3 f_tangentViewPos;
layout(location = 4) in vec3 f_tangentFragPos;
layout(location = 5) in vec3 f_fragNormal;

layout(location = 0) out vec4 FragColor;  // Output color

vec4 diffuseTexture() {
    if (material.diffuseIndex == -1)
    return vec4(material.diffuse, 1.0);
    return texture(material.textures, vec3(f_texCoords, float(material.diffuseIndex)));
}
vec4 specularTexture() {
    if (material.specularIndex == -1)
    return vec4(material.specular, 1.0);
    return texture(material.textures, vec3(f_texCoords, float(material.specularIndex)));
}
vec3 normalMap() {
    if (material.normalIndex == -1)
    return f_fragNormal;
    return normalize(texture(material.textures, vec3(f_texCoords, float(material.normalIndex))).rgb * 2.0 - 1.0);
}

vec4 ambient() {
    return vec4(light.ambient, 1.0);
}

vec4 diffuse(vec3 normal, vec3 lightDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = material.diffuse * light.diffuse * diff;
    return vec4(diffuse, 1.0);
}

const float kPi = 3.14159265359;

vec4 specular(vec3 normal, vec3 lightDir, vec3 viewDir) {
    if (material.specularIndex == -1)
    return vec4(vec3(0.0), 0.0);
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float energyConservation = (8.0 + material.shininess) / (8.0 * kPi);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    vec3 specular = energyConservation * 0.5 * material.specular * light.specular * spec;
    return vec4(specular, 1.0);
}

void main() {
    vec3 norm = normalMap();
    vec3 lightDir; vec3 viewDir;
    if (material.normalIndex == -1) {
        lightDir = normalize(-light.direction);
        viewDir = normalize(viewPos - f_fragPos);
    }
    else {
        lightDir = normalize(f_tangentLightDir);
        viewDir = normalize(f_tangentViewPos - f_tangentFragPos);
    }

    vec4 ambientLight = ambient() * diffuseTexture();
    vec4 diffuseLight = diffuse(norm, lightDir) * diffuseTexture();
    vec4 specularLight = specular(norm, lightDir, viewDir) * specularTexture();

    FragColor = ambientLight + diffuseLight + specularLight;
}