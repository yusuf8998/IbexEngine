#version 450

layout(location = 0) in vec3 v_position;  // Vertex position
layout(location = 1) in vec2 v_uv;        // Vertex uv
layout(location = 2) in vec3 v_normal;    // Vertex normal

layout(location = 0) out vec3 g_fragNormal;  // Normal to pass to fragment shader
layout(location = 1) out vec3 g_fragPos;     // Position to pass to fragment shader
layout(location = 2) out vec2 g_fragUV;      // uv to pass to fragment shader

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vec3 worldPosition = vec3(model * vec4(v_position, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(model)));

    g_fragPos = worldPosition;
    g_fragNormal = normalize(normalMatrix * v_normal);
    g_fragUV = v_uv;
    gl_Position = projection * view * vec4(worldPosition, 1.0);
}