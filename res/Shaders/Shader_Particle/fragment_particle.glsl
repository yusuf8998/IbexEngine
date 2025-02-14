#version 330 core

// Input from the vertex shader
in vec4 fragColor;        // Color of the particle

// Output to the framebuffer
out vec4 finalColor;      // Final color output

void main() {
    // Combine the texture color with the particle color (you can adjust the alpha for fading)
    finalColor = fragColor;
}