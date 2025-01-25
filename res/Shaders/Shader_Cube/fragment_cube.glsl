#version 450

layout(location = 0) in vec3 texCoords;

layout(location = 0) out vec4 color;

uniform samplerCube cubemap;

void main()
{
    color = texture(cubemap, texCoords);
}