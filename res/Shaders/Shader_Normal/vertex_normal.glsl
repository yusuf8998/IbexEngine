#version 330 core
layout(location = 0) in vec3 v_position;  // Vertex position
layout(location = 1) in vec2 v_uv;        // Vertex uv
layout(location = 2) in vec3 v_normal;    // Vertex normal
layout(location = 3) in vec3 v_tangent;   // Vertex tangent

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(view * model)));
    vs_out.normal = vec3(vec4(normalMatrix * v_normal, 0.0));
    gl_Position = view * model * vec4(v_position, 1.0); 
}