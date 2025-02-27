#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec2 fragUV;

uniform sampler2D image;

layout(location = 0) out vec4 finalColor;

void main() {
    finalColor = fragColor * texture(image, fragUV);
}