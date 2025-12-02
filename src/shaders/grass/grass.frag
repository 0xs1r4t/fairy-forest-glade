#version 330 core
in vec2 TexCoord;
in vec3 WorldPos;
in float HeightFactor;

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
    
    // Wind sway
    float sway = sin(WorldPos.x * 0.5 + time * 2.0) * 0.1;
    lightIntensity += sway * HeightFactor;
    
    // Height-based base colour
    vec3 baseColor = mix(GRASS_DARK, GRASS_MID, HeightFactor * 0.5);
    if (HeightFactor > 0.7) {
        baseColor = mix(GRASS_MID, GRASS_TIP, (HeightFactor - 0.7) / 0.3);
    }
    
    // Use smooth quantized shading
    vec3 colour = celShadeSmoothBands(lightIntensity, baseColor * 0.7, baseColor * 1.2, 4.0);
        
    FragColor = vec4(colour, alpha);
}