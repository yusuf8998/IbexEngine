#version 450

struct Material {
    sampler2D diffuseTexture;
    sampler2D specularTexture;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;
uniform vec3 viewPos;

layout(location = 0) in vec3 fragNormal;  // Normal from vertex shader
layout(location = 1) in vec3 fragPos;     // Position from vertex shader
layout(location = 2) in vec2 fragUV;      // UV from vertex shader

layout(location = 0) out vec4 FragColor;  // Output color

vec4 ambient() {
    return vec4(dirLight.ambient, 1.0);
}

vec4 diffuse(vec3 normal, vec3 lightDir) {
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff;
    return vec4(diffuse, 1.0);
}

vec4 specular(vec3 norm, vec3 lightDir) {
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = 0.5 * dirLight.specular * spec;
    return vec4(specular, 1.0);
}

void main() {
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(-dirLight.direction);

    vec4 ambientLight = ambient() * texture2D(material.diffuseTexture, fragUV);
    vec4 diffuseLight = diffuse(norm, lightDir) * texture2D(material.diffuseTexture, fragUV);
    vec4 specularLight = specular(norm, lightDir) * texture2D(material.specularTexture, fragUV);

    vec4 light = ambientLight + diffuseLight + specularLight;
    FragColor = light;
}