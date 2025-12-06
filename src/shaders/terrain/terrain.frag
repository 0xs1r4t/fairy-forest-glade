#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexColor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

// Get terrain color based on height
vec3 getTerrainColor(float height) {
    // Low (valleys) - dark mossy green
    if (height < 1.5) {
        return vec3(0.2, 0.3, 0.15);
    }
    // Mid-low (plains) - grass
    else if (height < 4.0) {
        return GRASS_DARK;
    }
    // Mid (gentle hills) - lighter grass
    else if (height < 7.0) {
        return GRASS_MID;
    }
    // High (hills) - brownish grass
    else if (height < 10.0) {
        return vec3(0.45, 0.5, 0.35);
    }
    // Peaks - rocky/grey
    else {
        return vec3(0.5, 0.5, 0.45);
    }
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Lighting (half-lambert)
    float NdotL = dot(norm, lightDir) * 0.5 + 0.5;
    
    // Get base color from height
    float height = FragPos.y;
    vec3 baseColor = getTerrainColor(height);
    
    // Apply cel-shading
    vec3 shadedColor = celShade4Band(
        NdotL,
        baseColor * 0.4,  // Deep shadow - darker
        baseColor * 0.65, // Shadow - darker
        baseColor * 0.85, // Mid - reduced from 1.0
        baseColor * 1.0   // Highlight - NO BOOST (was 1.15)
    );

    
    // Darken steep slopes (ambient occlusion)
    float slope = norm.y; // 1.0 = flat, 0.0 = cliff
    shadedColor *= mix(0.5, 1.0, slope);
    
    FragColor = vec4(shadedColor, 1.0);
}
