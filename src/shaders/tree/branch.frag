#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 lightDir;
uniform vec3 viewPos;

void main() {
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - FragPos);
    
    float NdotL = dot(N, L) * 0.5 + 0.5;
    float height = clamp(FragPos.y * 0.2 + 0.5, 0.0, 1.0);
    
    // Height-based bark colour
    vec3 baseColor = mix(DARK_BARK, MID_BARK, height);
    
    // Quantized shading (function from NPR library)
    vec3 branchColor = celShadeQuantized(NdotL, baseColor * 0.6, LIGHT_BARK, 6.0);
    
    // Rim lighting
    float rim = pow(1.0 - max(dot(V, N), 0.0), 3.0) * 0.3;
    branchColor += vec3(rim);
    
    FragColor = vec4(branchColor, 1.0);
}
