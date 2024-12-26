#version 450

layout(triangles) in;        // Input primitive is a triangle
layout(triangle_strip, max_vertices = 3) out; // Output primitive is also a triangle

layout(location = 0) in vec3 g_fragNormal[3];  // Normal to pass to fragment shader
layout(location = 1) in vec3 g_fragPos[3];     // Position to pass to fragment shader
layout(location = 2) in vec2 g_fragUV[3];      // uv to pass to fragment shader

layout(location = 0) out vec3 f_fragNormal;  // Normal from vertex shader
layout(location = 1) out vec3 f_fragPos;     // Position from vertex shader
layout(location = 2) out vec2 f_fragUV;      // UV from vertex shader

void main() {
    // Pass through the input vertices to the output
    f_fragNormal = g_fragNormal[0];
    f_fragPos = g_fragPos[0];
    f_fragUV = g_fragUV[0];
    gl_Position = gl_in[0].gl_Position;
    EmitVertex(); // Emit first vertex

    f_fragNormal = g_fragNormal[1];
    f_fragPos = g_fragPos[1];
    f_fragUV = g_fragUV[1];
    gl_Position = gl_in[1].gl_Position;
    EmitVertex(); // Emit second vertex

    f_fragNormal = g_fragNormal[2];
    f_fragPos = g_fragPos[2];
    f_fragUV = g_fragUV[2];
    gl_Position = gl_in[2].gl_Position;
    EmitVertex(); // Emit third vertex

    EndPrimitive(); // End the primitive (triangle)
}
