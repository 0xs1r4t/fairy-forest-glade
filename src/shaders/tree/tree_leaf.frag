#version 330 core
in vec2 TexCoord;
in vec3 CustomNormal;
in vec3 WorldPos;
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
    
    // palette set using colour library
    vec3 shadedColor = celShade4Band(NdotL, DARK_FOREST_GREEN, DEEP_GREEN, MID_GREEN, BRIGHT_GREEN);
    
    // Alternative - try different greens:
    // vec3 shadedColor = celShade3Band(NdotL, MID_GREEN, BRIGHT_GREEN, LIME_GREEN);
    
    // Ambient occlusion
    float aoFactor = smoothstep(-1.0, 1.0, WorldPos.y);
    shadedColor *= mix(0.6, 1.0, aoFactor);
    
    FragColor = vec4(shadedColor, alpha);
}