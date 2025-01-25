#version 450

layout(triangles) in;        // Input primitive is a triangle
layout(triangle_strip, max_vertices = 3) out; // Output primitive is also a triangle

layout(location = 0) in vec3 g_fragPos[3];
layout(location = 1) in vec2 g_texCoords[3];
layout(location = 2) in vec3 g_tangentLightDir[3];
layout(location = 3) in vec3 g_tangentViewPos[3];
layout(location = 4) in vec3 g_tangentFragPos[3];
layout(location = 5) in vec3 g_fragNormal[3];

layout(location = 0) out vec3 f_fragPos;
layout(location = 1) out vec2 f_texCoords;
layout(location = 2) out vec3 f_tangentLightDir;
layout(location = 3) out vec3 f_tangentViewPos;
layout(location = 4) out vec3 f_tangentFragPos;
layout(location = 5) out vec3 f_fragNormal;

void main() {
    // Pass through the input vertices to the output
    f_fragPos = g_fragPos[0];
    f_texCoords = g_texCoords[0];
    f_tangentLightDir = g_tangentLightDir[0];
    f_tangentViewPos = g_tangentViewPos[0];
    f_tangentFragPos = g_tangentFragPos[0];
    f_fragNormal = g_fragNormal[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex(); // Emit first vertex

    f_fragPos = g_fragPos[1];
    f_texCoords = g_texCoords[1];
    f_tangentLightDir = g_tangentLightDir[1];
    f_tangentViewPos = g_tangentViewPos[1];
    f_tangentFragPos = g_tangentFragPos[1];
    f_fragNormal = g_fragNormal[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex(); // Emit second vertex

    f_fragPos = g_fragPos[2];
    f_texCoords = g_texCoords[2];
    f_tangentLightDir = g_tangentLightDir[2];
    f_tangentViewPos = g_tangentViewPos[2];
    f_tangentFragPos = g_tangentFragPos[2];
    f_fragNormal = g_fragNormal[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex(); // Emit third vertex

    EndPrimitive(); // End the primitive (triangle)
}
