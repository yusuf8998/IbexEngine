#version 450
layout (location = 0) in vec3 v_position;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(v_position, 1.0);
}