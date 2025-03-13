#version 450

struct Material {
    sampler2DArray textures;
    int diffuseIndex;
    int specularIndex;
    int normalIndex;
    int displacementIndex;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

layout(location = 0) in vec3 v_position;  // Vertex position
layout(location = 1) in vec2 v_uv;        // Vertex uv
layout(location = 2) in vec3 v_normal;    // Vertex normal
layout(location = 3) in vec3 v_tangent;   // Vertex tangent

layout(location = 0) out vec3 g_fragPos;
layout(location = 1) out vec2 g_texCoords;
layout(location = 2) out mat3 g_TBN; // 2, 3, 4
layout(location = 5) out int g_displaced;
layout(location = 6) out vec3 g_fragNormal;

uniform Material material;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    vec4 newPos = vec4(v_position, 1.0);
    g_displaced = 0;
    if (material.displacementIndex != -1) {
        float newHeight = texture(material.textures, vec3(v_uv, float(material.displacementIndex))).r * 0.05;
        newPos += vec4(v_normal, 0.0) * newHeight;
        g_displaced = 1;
    }

    g_fragPos = vec3(model * newPos);
    g_texCoords = v_uv;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 T = normalize(normalMatrix * v_tangent);
    vec3 N = normalize(normalMatrix * v_normal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    g_fragNormal = normalMatrix * v_normal;

    mat3 TBN = transpose(mat3(T, B, N));    
    g_TBN = TBN;

    gl_Position = projection * view * model * newPos;
}