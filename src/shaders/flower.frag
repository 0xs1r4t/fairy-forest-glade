#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D flowerTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec4 texColor = texture(flowerTexture, TexCoords);
    
    // Alpha cutout
    if (texColor.a < 0.1) {
        discard;
    }
    
    // Simple lighting - flowers are brighter
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Higher ambient for flowers (they're more vibrant)
    float ambient = 0.5;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Combine
    vec3 result = (ambient + diff) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
}