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

// Environment map
uniform sampler2D environmentMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    // Basic lighting
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Environment mapping
    vec3 envColor = texture(environmentMap, SampleSphericalMap(norm)).rgb;
    vec3 envAmbient = 0.3 * envColor;
    
    // Initialize result with ambient
    vec3 result = envAmbient * materialColor;
    
    // LIGHT 1: Main scene light
    {
        vec3 lightDir = normalize(lightPos - FragPos);
        
        // Diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColor;
        
        // Specular
        float specularStrength = 0.5;
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        vec3 specular = specularStrength * spec * lightColor;
        
        result += (diffuse * materialColor + specular);
    }

    // LIGHT 2: Fairy light (point light above fairy)
    {
        vec3 fairyDir = normalize(fairyLightPos - FragPos);
        float distance = length(fairyLightPos - FragPos);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
        
        // Diffuse
        float diff = max(dot(norm, fairyDir), 0.0);
        vec3 diffuse = diff * fairyLightColor * attenuation;
        
        result += diffuse * materialColor;
    }
    
    // Environment reflection
    vec3 R = reflect(-viewDir, norm);
    vec3 envReflection = texture(environmentMap, SampleSphericalMap(R)).rgb;
    float reflectivity = max(0.0, (materialShininess - 1.0) / 128.0);
    result += reflectivity * envReflection;
    
    FragColor = vec4(result, 1.0);
}