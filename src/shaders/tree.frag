#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D treeTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main() {
    vec4 texColor = texture(treeTexture, TexCoords);
    
    // Alpha cutout
    if (texColor.a < 0.1) {
        discard;
    }
    
    // Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    float ambient = 0.3;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Trees have more pronounced shadows
    vec3 result = (ambient + diff * 0.7) * texColor.rgb;
    
    FragColor = vec4(result, texColor.a);
}