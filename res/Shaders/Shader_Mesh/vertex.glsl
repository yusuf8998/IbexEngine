#version 450

layout(location = 0) in vec3 v_position;  // Vertex position
layout(location = 1) in vec2 v_uv;    // Vertex uv
layout(location = 2) in vec3 v_normal;    // Vertex normal

layout(location = 0) out vec3 fragNormal;  // Normal to pass to fragment shader
layout(location = 1) out vec3 fragPos;     // Position to pass to fragment shader
layout(location = 2) out vec2 fragUV;     // uv to pass to fragment shader

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(v_position, 1);
    fragPos = v_position;
    fragNormal = v_normal;
    fragUV = v_uv;
}