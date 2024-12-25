#version 450

struct Material
{
    sampler2D diffuseTexture;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform DirLight dirLight;

layout(location = 0) in vec3 fragNormal;  // Normal from vertex shader
layout(location = 1) in vec3 fragPos;     // Position from vertex shader
layout(location = 2) in vec2 fragUV;      // UV from vertex shader

layout(location = 0) out vec4 FragColor;  // Output color

vec4 ambient()
{
    return vec4(dirLight.ambient * material.diffuse, 1.0);
}

vec4 diffuse(vec3 normal, vec3 lightDir)
{
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = dirLight.diffuse * diff * material.diffuse;
    return vec4(diffuse, 1.0);
}

void main()
{
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(-dirLight.direction);

    vec4 ambientLight = ambient();
    vec4 diffuseLight = diffuse(norm, lightDir);

    vec4 light = ambientLight + diffuseLight;
    FragColor = light * texture(material.diffuseTexture, fragUV);
}