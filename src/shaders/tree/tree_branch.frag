#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 viewPos;

// Cel-shading for branches (brown tones)
vec3 getBranchColor(float NdotL, float height) {
    // Height-based color variation (darker at base, lighter at tips)
    vec3 darkBrown = vec3(0.25, 0.15, 0.10);
    vec3 midBrown = vec3(0.40, 0.25, 0.15);
    vec3 lightBrown = vec3(0.55, 0.35, 0.20);
    
    // Cel-shading bands
    if(NdotL > 0.7) return mix(lightBrown, vec3(0.7, 0.5, 0.3), height);
    else if(NdotL > 0.3) return mix(midBrown, lightBrown, height * 0.5);
    else return mix(darkBrown, midBrown, height * 0.3);
}

void main() {
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - FragPos);
    
    // Basic lighting
    float NdotL = dot(N, L) * 0.5 + 0.5; // Half-lambert
    
    // Height factor for color variation (0 = base, 1 = top)
    float height = clamp(FragPos.y * 0.2 + 0.5, 0.0, 1.0);
    
    vec3 branchColor = getBranchColor(NdotL, height);
    
    // Rim lighting for depth
    float rim = 1.0 - max(dot(V, N), 0.0);
    rim = pow(rim, 3.0) * 0.3;
    branchColor += vec3(rim);
    
    // Ambient occlusion approximation (darker in crevices)
    float ao = smoothstep(0.0, 1.0, NdotL);
    branchColor *= mix(0.6, 1.0, ao);
    
    FragColor = vec4(branchColor, 1.0);
}
