#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// Material properties
uniform vec3 materialColor;
uniform float materialShininess;
uniform bool isTerrain; // New flag to identify terrain

void main()
{    
    vec3 finalMaterialColor = materialColor;
    float finalShininess = materialShininess;
    
    // Apply different materials based on object type
    if (isTerrain) {
        // Height-based coloring for terrain
        float height = FragPos.y;
        
        // Create different color zones based on height
        if (height < -3.0) {
            finalMaterialColor = vec3(0.2, 0.4, 0.1); // Dark green (low areas)
        } else if (height < 0.0) {
            finalMaterialColor = vec3(0.3, 0.6, 0.2); // Grass green
        } else if (height < 2.0) {
            finalMaterialColor = vec3(0.5, 0.55, 0.3); // Light green/yellow
        } else if (height < 4.0) {
            finalMaterialColor = vec3(0.6, 0.5, 0.4); // Rocky brown
        } else {
            finalMaterialColor = vec3(0.9, 0.9, 0.95); // Snowy white
        }
        
        // Terrain is less shiny (more rough)
        finalShininess = 8.0;
        
        // Optional: Add slope-based coloring
        vec3 up = vec3(0.0, 1.0, 0.0);
        float slope = dot(normalize(Normal), up);
        
        // Steeper slopes get rocky color
        if (slope < 0.7) {
            finalMaterialColor = mix(finalMaterialColor, vec3(0.5, 0.4, 0.35), 0.5);
        }
    }
    
    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), finalShininess);
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 result = (ambient + diffuse + specular) * finalMaterialColor;
    FragColor = vec4(result, 1.0);
}