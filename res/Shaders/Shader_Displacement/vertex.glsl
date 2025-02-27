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
struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

layout(location = 0) in vec3 v_position;  // Vertex position
layout(location = 1) in vec2 v_uv;        // Vertex uv
layout(location = 2) in vec3 v_normal;    // Vertex normal
layout(location = 3) in vec3 v_tangent;   // Vertex tangent

layout(location = 0) out vec3 g_fragPos;
layout(location = 1) out vec2 g_texCoords;
layout(location = 2) out vec3 g_tangentLightDir;
layout(location = 3) out vec3 g_tangentViewPos;
layout(location = 4) out vec3 g_tangentFragPos;
layout(location = 5) out vec3 g_fragNormal;

uniform Light light;
uniform Material material;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 viewPos;

void main() {
    vec4 newPos = vec4(v_position, 1.0);
    if (material.displacementIndex != -1)
    {
        float newHeight = texture(material.textures, vec3(v_uv, float(material.displacementIndex))).r * 0.05;
        newPos += vec4(v_normal, 0.0) * newHeight;
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
    g_tangentLightDir = TBN * -light.direction;
    g_tangentViewPos  = TBN * viewPos;
    g_tangentFragPos  = TBN * g_fragPos;

    gl_Position = projection * view * model * newPos;
}