#version 450

layout(points) in;
layout(triangle_strip, max_vertices = 6) out;

layout(location = 0) in vec3 g_midPoint[1];

layout(location = 0) out vec2 f_uv;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    gl_Position = projection * ((view * vec4(g_midPoint[0], 1.0)) + (model * vec4(vec3(-1.0, -1.0, 0.0), 0.0)));
    f_uv = vec2(0.0, 0.0);
    EmitVertex();

    gl_Position = projection * ((view * vec4(g_midPoint[0], 1.0)) + (model * vec4(vec3(1.0, -1.0, 0.0), 0.0)));
    f_uv = vec2(1.0, 0.0);
    EmitVertex();

    gl_Position = projection * ((view * vec4(g_midPoint[0], 1.0)) + (model * vec4(vec3(1.0, 1.0, 0.0), 0.0)));
    f_uv = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();

    gl_Position = projection * ((view * vec4(g_midPoint[0], 1.0)) + (model * vec4(vec3(1.0, 1.0, 0.0), 0.0)));
    f_uv = vec2(1.0, 1.0);
    EmitVertex();

    gl_Position = projection * ((view * vec4(g_midPoint[0], 1.0)) + (model * vec4(vec3(-1.0, 1.0, 0.0), 0.0)));
    f_uv = vec2(0.0, 1.0);
    EmitVertex();

    gl_Position = projection * ((view * vec4(g_midPoint[0], 1.0)) + (model * vec4(vec3(-1.0, -1.0, 0.0), 0.0)));
    f_uv = vec2(0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}