#version 450
layout(location = 0) in vec3 v_position;  // Vertex position
layout(location = 1) in vec2 v_uv;        // Vertex uv
layout(location = 2) in vec3 v_normal;    // Vertex normal
layout(location = 3) in vec3 v_tangent;   // Vertex tangent

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

layout(location = 0) out vec3 g_normal;

uniform mat4 view;
uniform mat4 model;

uniform Material material;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));

    vec4 newPos = vec4(v_position, 1.0);
    if (material.displacementIndex != -1) {
        float newHeight = texture(material.textures, vec3(v_uv, float(material.displacementIndex))).r * 0.05;
        newPos += vec4(v_normal, 0.0) * newHeight;
    }

    g_normal = vec3(vec4(normalMatrix * v_normal, 0.0));
    gl_Position = view * model * newPos;
}