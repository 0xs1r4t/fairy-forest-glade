#version 330 core
out vec4 FragColor;

in vec3 FragColor;
in float Glow;

void main() {
    // Emissive glow - no lighting calculations needed
    vec3 emissive = FragColor * Glow * 2.0; // Bright!
    
    FragColor = vec4(emissive, 1.0);
}