#version 450

layout(location = 0) in vec2 f_uv;

layout(location = 0) out vec4 FragColor;

uniform sampler2D image;

void main()
{
    FragColor = texture(image, f_uv);
}