#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

// Material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// Firefly lights
uniform vec3 fireflyPositions[8];
uniform vec3 fireflyColors[8];
uniform int numFireflies;

void main() {
    // Sample textures with fallback colors
    vec4 albedo = texture(texture_diffuse1, TexCoords);
    
    // CRITICAL FIX: If texture is missing/black, use a default color
    if (length(albedo.rgb) < 0.01) {
        // Default bark brown for trunk, green for leaves
        albedo = vec4(0.4, 0.3, 0.2, 1.0); // Brown
    }
    
    // More lenient alpha test (or remove it for debugging)
    if (albedo.a < 0.1) {
        discard;
    }
    
    // Use geometric normal if normal map is missing
    vec3 normalMap = texture(texture_normal1, TexCoords).rgb;
    vec3 normal;
    if (length(normalMap) < 0.01) {
        normal = normalize(Normal); // Use vertex normal
    } else {
        normalMap = normalMap * 2.0 - 1.0;
        normal = normalize(TBN * normalMap);
    }
    
    // BRIGHTER ambient so you can see it!
    vec3 ambient = vec3(0.3) * albedo.rgb; // Increased from 0.05
    vec3 result = ambient;
    
    // Moonlight
    {
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * 0.5; // Increased from 0.15
        result += diffuse * albedo.rgb;
    }
    
    // Firefly lights
    for (int i = 0; i < numFireflies; i++) {
        vec3 fireflyDir = normalize(fireflyPositions[i] - FragPos);
        float distance = length(fireflyPositions[i] - FragPos);
        
        if (distance < 5.0) {
            float attenuation = 1.0 / (1.0 + 0.3 * distance + 0.1 * distance * distance);
            float diff = max(dot(normal, fireflyDir), 0.0);
            vec3 diffuse = diff * fireflyColors[i] * attenuation * 0.3;
            result += diffuse * albedo.rgb;
        }
    }
    
    FragColor = vec4(result, albedo.a);
}