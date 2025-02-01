#version 450

layout(location = 0) in vec2 f_uv;

layout(location = 0) out vec3 FragColor;

uniform sampler2D image;

void main()
{
    FragColor = texture(image, f_uv).rgb;
}