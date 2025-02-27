#version 450

layout(location = 0) in vec3 v_position;

layout(location = 0) out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    mat4 rot_view = mat4(mat3(view));
    vec4 pos = projection * rot_view * vec4(v_position, 1);
    gl_Position = pos.xyww;
    texCoords = v_position;
}