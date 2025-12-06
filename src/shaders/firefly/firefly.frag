#version 330 core
out vec4 FragColor;

in vec3 Color;
in float Glow;
in vec2 TexCoord;

uniform float time;

void main() {
    vec2 center = vec2(0.5);
    float dist = length(TexCoord - center);
    
    // Soft falloff
    float alpha = 1.0 - smoothstep(0.0, 0.5, dist);
    alpha = pow(alpha, 2.5); // Slightly sharper
    
    // BRIGHTER glow
    vec3 emissive = Color * Glow * 2.5; // Increased brightness
    
    // Subtle center highlight
    float centerGlow = exp(-dist * 12.0) * 0.5;
    emissive += Color * centerGlow;
    
    FragColor = vec4(emissive, alpha * 0.9); // Slightly more opaque
}
