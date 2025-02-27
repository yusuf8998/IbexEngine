#version 450

layout(location = 0) in vec3 v_position;  // Vertex position

layout(location = 0) out vec3 g_midPoint;

uniform mat4 model;

void main()
{
    g_midPoint = (model * vec4(v_position, 1.0)).xyz;
    gl_Position = vec4(g_midPoint, 1.0);
}