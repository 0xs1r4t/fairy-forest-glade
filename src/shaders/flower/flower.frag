#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
flat in int TexIndex;
in float HeightFactor; // NEW
in float WindInfluence; // NEW

uniform sampler2D flowerTexture0; // Flower1.png
uniform sampler2D flowerTexture1; // Flower2.png

void main() {
    // Sample correct texture based on index
    vec4 texColor;
    if (TexIndex == 0) {
        texColor = texture(flowerTexture0, TexCoords);
    } else {
        texColor = texture(flowerTexture1, TexCoords);
    }
    
    // Discard fully transparent pixels
    if (texColor.a < 0.01) {
        discard;
    }
    
    // ===== WIND EFFECTS =====
    // Subtle brightness variation from wind
    float windShimmer = WindInfluence * HeightFactor * 0.1; // Subtle
    
    // Apply shimmer to flower colors
    vec3 finalColor = texColor.rgb * (1.0 + windShimmer);
    
    FragColor = vec4(finalColor, texColor.a);
}
