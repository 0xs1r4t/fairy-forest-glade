#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 fairyLightPos;
uniform vec3 fairyLightColor;
uniform vec3 materialColor;
uniform float materialShininess;

// Firefly lights
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

// Ambient occlusion
float calculateAO(vec3 normal, vec3 viewDir) {
    float ao = dot(normal, viewDir);
    ao = ao * 0.5 + 0.5;
    ao = pow(ao, 0.5);
    return mix(0.4, 1.0, ao);
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    float ao = calculateAO(norm, viewDir);
    
    // Start almost black
    vec3 result = vec3(0.0);
    
    // ===== MOONLIGHT - BARELY THERE =====
    vec3 moonDir = normalize(lightPos);
    float moonDiff = max(dot(norm, moonDir), 0.0);
    
    // CUT TO 0.08
    vec3 moonDiffuse = moonDiff * lightColor * 0.08 * ao;
    
    // CUT rim to 0.04
    float rimLight = pow(1.0 - max(dot(viewDir, norm), 0.0), 3.0);
    rimLight *= max(dot(norm, moonDir), 0.0);
    vec3 moonRim = rimLight * lightColor * 0.04;
    
    result += (moonDiffuse + moonRim) * materialColor;
    
    // ===== FIREFLY LIGHTS - SUBTLE =====
    for (int i = 0; i < numFireflies; i++) {
        vec3 fireflyDir = normalize(fireflyPositions[i] - FragPos);
        float distance = length(fireflyPositions[i] - FragPos);
        
        if (distance < 4.0) { // REDUCED range from 5.0
            // Stronger falloff
            float attenuation = 1.0 / (1.0 + 0.7 * distance + 0.5 * distance * distance);
            
            float diff = max(dot(norm, fireflyDir), 0.0);
            // CUT TO 0.35
            vec3 diffuse = diff * fireflyColors[i] * attenuation * 0.35;
            
            // CUT TO 0.12
            vec3 ambientFirefly = fireflyColors[i] * attenuation * 0.12;
            
            result += (diffuse + ambientFirefly) * materialColor;
        }
    }
    
    // Tone mapping
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}
