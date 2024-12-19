#version 450

layout(location = 0) in vec3 v_color;
layout(location = 1) in vec2 v_uv;

layout(location = 0) out vec3 f_color;

uniform sampler2D u_texture;

void main()
{
    f_color = v_color * vec3(texture(u_texture, v_uv));
}