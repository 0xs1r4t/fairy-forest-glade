#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;          // Main moonlight
uniform vec3 lightColor;
uniform vec3 fairyLightPos;     // Fairy glow
uniform vec3 fairyLightColor;
uniform vec3 materialColor;
uniform float materialShininess;

// Firefly lights (we'll pass the closest ones)
uniform vec3 fireflyPositions[8];
uniform vec3 fireflyColors[8];
uniform int numFireflies;

// Environment map
uniform sampler2D environmentMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Very dark environment for night
    vec3 envColor = texture(environmentMap, SampleSphericalMap(norm)).rgb;
    vec3 envAmbient = 0.05 * envColor; // Much darker!
    
    vec3 result = envAmbient * materialColor;
    
    // MOONLIGHT (weak directional light)
    {
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor * 0.15; // Very weak moonlight
        
        result += diffuse * materialColor;
    }

    // FAIRY LIGHT (point light)
    {
        vec3 fairyDir = normalize(fairyLightPos - FragPos);
        float distance = length(fairyLightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.05 * distance + 0.01 * distance * distance);
        
        float diff = max(dot(norm, fairyDir), 0.0);
        vec3 diffuse = diff * fairyLightColor * attenuation * 1.5; // Brighter fairy glow
        
        result += diffuse * materialColor;
    }
    
    // FIREFLY LIGHTS (multiple small point lights)
    for (int i = 0; i < numFireflies; i++) {
        vec3 fireflyDir = normalize(fireflyPositions[i] - FragPos);
        float distance = length(fireflyPositions[i] - FragPos);
        
        // Fireflies have very short range
        if (distance < 3.0) {
            float attenuation = 1.0 / (1.0 + 0.5 * distance + 0.3 * distance * distance);
            
            float diff = max(dot(norm, fireflyDir), 0.0);
            vec3 diffuse = diff * fireflyColors[i] * attenuation * 0.5;
            
            result += diffuse * materialColor;
        }
    }
    
    FragColor = vec4(result, 1.0);
}