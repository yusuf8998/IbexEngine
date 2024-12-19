#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec3 v_color;
layout(location = 1) out vec2 v_uv;

void main() {
	gl_Position = vec4(position, 1.0);
	v_color = color;
    v_uv = uv;
}