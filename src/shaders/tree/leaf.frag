#version 330 core
in vec2 TexCoord;
in vec3 CustomNormal;
in vec3 WorldPos;
in float WindInfluence;
flat in int TexIndex;

out vec4 FragColor;

uniform sampler2D leafTexture0;
uniform sampler2D leafTexture1;
uniform sampler2D leafTexture2;
uniform sampler2D leafTexture3;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 ambientColor;

void main() {
    // Sample texture
    vec4 texColor;
    if(TexIndex == 0) texColor = texture(leafTexture0, TexCoord);
    else if(TexIndex == 1) texColor = texture(leafTexture1, TexCoord);
    else if(TexIndex == 2) texColor = texture(leafTexture2, TexCoord);
    else texColor = texture(leafTexture3, TexCoord);
    
    float alpha = texColor.r;
    if(alpha < 0.3) discard;
    
    // Lighting
    vec3 N = normalize(CustomNormal);
    vec3 L = normalize(-lightDir);
    float NdotL = dot(N, L) * 0.5 + 0.5;
    
    // Cel-shaded color (function from NPR library)
    vec3 shadedColor = celShadeSmoothBands(NdotL, DARK_FOREST_GREEN, LEAF_MID, 6.0);
    
    // ===== WIND SHIMMER EFFECT =====
    // Subtle brightness variation from wind (leaves catching light)
    float windShimmer = WindInfluence * 0.1;
    shadedColor += vec3(windShimmer * 0.05, windShimmer * 0.08, windShimmer * 0.05);
    
    // Slight color shift when wind is strong (more yellow-green)
    vec3 windTint = vec3(0.1, 0.15, 0.05) * WindInfluence * 0.15;
    shadedColor = mix(shadedColor, shadedColor + windTint, WindInfluence * 0.3);
    
    // AO
    float aoFactor = smoothstep(0.0, 2.0, length(WorldPos));
    shadedColor *= mix(0.5, 1.0, aoFactor);
    
    FragColor = vec4(shadedColor, alpha);
}
