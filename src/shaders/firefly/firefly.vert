#version 330 core
layout (location = 0) in vec3 aPos;       // Sphere vertex position
layout (location = 1) in vec3 aOffset;    // Instance position
layout (location = 2) in vec3 aColor;     // Instance color

uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 FragColor;
out float Glow;

void main() {
    // Pulsing size effect
    float pulse = sin(time * 3.0 + aOffset.x + aOffset.z) * 0.5 + 0.5;
    float scale = 1.0 + pulse * 0.3;
    
    vec3 worldPos = aPos * scale + aOffset;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    FragColor = aColor;
    Glow = 0.8 + pulse * 0.2; // Pulsing glow intensity
}