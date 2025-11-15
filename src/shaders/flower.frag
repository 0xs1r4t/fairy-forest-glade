#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
in vec3 LocalPos;

uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // Determine if this is petal or stem based on height
    bool isPetal = LocalPos.y > 0.0;
    
    // Simple flat colors
    vec3 petalColor = vec3(0.95, 0.4, 0.6);  // Pink
    vec3 stemColor = vec3(0.3, 0.5, 0.2);     // Green
    
    vec3 baseColor = isPetal ? petalColor : stemColor;
    
    // Basic lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    vec3 ambient = 0.5 * baseColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * baseColor;
    
    vec3 result = ambient + diffuse;
    
    FragColor = vec4(result, 1.0);
}
