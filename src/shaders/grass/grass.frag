#version 330 core
in vec2 TexCoord;
in vec3 WorldPos;
in float HeightFactor;
in float WindInfluence;

out vec4 FragColor;

uniform sampler2D grassTexture;
uniform vec3 lightDir;
uniform float time;
uniform vec3 ambientColor;

void main() {
    vec4 texColor = texture(grassTexture, TexCoord);
    float alpha = texColor.r;
    if (alpha < 0.2) discard;
    
    vec3 normal = vec3(0.0, 1.0, 0.0);
    float lightIntensity = dot(normalize(-lightDir), normal) * 0.5 + 0.5;
    
    // ===== WIND SHIMMER =====
    // Dynamic lighting from wind movement (grass catching light)
    float windShimmer = WindInfluence * HeightFactor * 0.15;
    lightIntensity += windShimmer;
    
    // Color shift when wind is strong (more yellow-green at tips)
    vec3 windTint = vec3(0.1, 0.15, 0.05) * WindInfluence * HeightFactor * 0.2;
    
    // Height-based base colour
    vec3 baseColor = mix(GRASS_DARK, GRASS_MID, HeightFactor * 0.5);
    if (HeightFactor > 0.7) {
        baseColor = mix(GRASS_MID, GRASS_TIP, (HeightFactor - 0.7) / 0.3);
    }
    
    // Add wind tint
    baseColor += windTint;
    
    // Cel-shaded with smooth bands (function from NPR library)
    vec3 colour = celShadeSmoothBands(lightIntensity, baseColor * 0.7, baseColor * 1.2, 4.0);
    
    // Subtle specular highlight on wind-blown tips
    if (HeightFactor > 0.8 && WindInfluence > 0.6) {
        colour += vec3(0.1, 0.12, 0.08) * (HeightFactor - 0.8) * 2.0;
    }
    
    FragColor = vec4(colour, alpha);
}
