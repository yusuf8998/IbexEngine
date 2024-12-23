#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 FragColor;

struct Material
{
    vec3 diffuse;
    vec3 specular;
    float shininess;
    sampler2D diffuseTexture;
};

uniform Material material;

void main()
{
    FragColor = vec4(material.diffuse, 1.0) * texture(material.diffuseTexture, fragUV);
    // FragColor = vec4( vec3(1.0), 0.0 );
}