#version 330 core

// Input vertex data from the buffer
layout(location = 0) in vec3 position;   // Quad position
layout(location = 1) in vec3 instancePos;  // Per-instance position
layout(location = 2) in vec3 instanceVel;  // Per-instance velocity
layout(location = 3) in float instanceSize; // Per-instance size
layout(location = 4) in vec4 instanceColor; // Per-instance color
layout(location = 5) in float instanceLifetime; // Per-instance lifetime

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Output to fragment shader
out vec4 fragColor;

void main() {
    vec3 scale, translation;

    scale.x = length(vec3(model[0]));
    scale.y = length(vec3(model[1]));
    scale.z = length(vec3(model[2]));

    translation = vec3(model[3]);

    mat4 new_model = mat4(1.0);
    new_model[0] = vec4(scale.x, 0.0, 0.0, 0.0);
    new_model[1] = vec4(0.0, scale.y, 0.0, 0.0);
    new_model[2] = vec4(0.0, 0.0, scale.z, 0.0);
    new_model[3] = vec4(translation, 1.0);

    // Apply instance data to particle position
    vec3 newInsPosition = instancePos + instanceVel * instanceLifetime;
    vec3 newQuadPosition = position * instanceSize;

    // Set the particle position in the world (or camera) space
    gl_Position = projection * ((view * vec4(newInsPosition, 1.0)) + (new_model * vec4(newQuadPosition, 0.0)));
    // Pass the color and size to the fragment shader
    fragColor = instanceColor;
}