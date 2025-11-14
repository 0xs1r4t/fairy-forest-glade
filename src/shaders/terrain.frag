#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexColor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main()
{
    // Use vertex color (procedurally generated in terrain.cpp)
    vec3 baseColor = VertexColor;
    
    // Lighting calculations
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * baseColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * baseColor;
    
    // Specular (subtle for terrain)
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);  // Low shininess
    vec3 specular = 0.1 * spec * lightColor;
    
    // Combine
    vec3 result = ambient + diffuse + specular;
    
    FragColor = vec4(result, 1.0);
}