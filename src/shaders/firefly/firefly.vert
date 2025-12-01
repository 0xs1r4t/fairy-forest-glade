#version 330 core
layout (location = 0) in vec3 aPos;       // Sphere vertex position
layout (location = 1) in vec3 aOffset;    // Instance position
layout (location = 2) in vec3 aColor;     // Instance color

uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 Color;
out float Glow;

void main() {
    // Stronger pulsing effect
    float pulse = sin(time * 3.0 + aOffset.x + aOffset.z) * 0.5 + 0.5;
    float scale = 1.5 + pulse * 0.8; // BIGGER pulsing
    
    vec3 worldPos = aPos * scale + aOffset;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    Color = aColor;
    Glow = 0.9 + pulse * 0.1; // Consistent bright glow
}