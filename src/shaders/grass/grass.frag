#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D grassTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec4 texColor = texture(grassTexture, TexCoords);
    
    // Alpha cutout - discard fully transparent pixels
    if (texColor.a < 0.1) {
        discard;
    }
    
    // Simple lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    float ambient = 0.4;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Combine
    vec3 result = (ambient + diff) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
}