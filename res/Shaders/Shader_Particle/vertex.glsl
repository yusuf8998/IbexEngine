#version 450

// Input vertex data from the buffer
layout(location = 0) in vec3 position;   // Quad position
layout(location = 1) in vec3 instancePos;  // Per-instance position
layout(location = 2) in vec3 instanceVel;  // Per-instance velocity
layout(location = 3) in vec3 instanceAcc;  // Per-instance acceleration
layout(location = 4) in float instanceSize; // Per-instance size
layout(location = 5) in vec4 instanceColor; // Per-instance color
layout(location = 6) in float instanceLifetime; // Per-instance lifetime

// Uniforms
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// Output to fragment shader
layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 fragUV;

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
    vec3 newVel = instanceVel + (instanceAcc * instanceLifetime);
    vec3 newInsPosition = instancePos + (newVel * instanceLifetime);
    vec3 newQuadPosition = position * instanceSize;

    fragUV = vec2(position.x + 0.5, position.y + 0.5);

    // Set the particle position in the world (or camera) space
    gl_Position = projection * ((view * vec4(newInsPosition, 1.0)) + (new_model * vec4(newQuadPosition, 0.0)));
    // Pass the color and size to the fragment shader
    fragColor = instanceColor;
}