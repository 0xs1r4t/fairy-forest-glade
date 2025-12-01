#version 330 core
out vec4 FragColor;

in vec3 Color;
in float Glow;

void main() {
    // SUPER BRIGHT emissive glow
    vec3 emissive = Color * Glow * 5.0; // CHANGED from 2.0 to 5.0!
    
    FragColor = vec4(emissive, 1.0);
}