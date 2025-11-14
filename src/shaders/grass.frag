#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D grassTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;

// Rotate UV coordinates
vec2 rotateUV(vec2 uv, float rotation)
{
    vec2 centered = uv - 0.5;
    float cosAngle = cos(rotation);
    float sinAngle = sin(rotation);
    
    vec2 rotated;
    rotated.x = centered.x * cosAngle - centered.y * sinAngle;
    rotated.y = centered.x * sinAngle + centered.y * cosAngle;
    
    return rotated + 0.5;
}

void main()
{
    float pi = 3.14159;
    float rotation = 3.0 * pi / 2.0;  // 270 degrees
    vec2 rotatedUV = rotateUV(TexCoords, rotation);
    
    // Sample texture with rotated UVs
    vec4 texColor = texture(grassTexture, rotatedUV);
    
    // Calculate brightness from texture (UNCOMMENTED!)
    float brightness = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    float alpha = brightness;  // Use brightness as alpha
    
    // Discard fully transparent pixels (black areas)
    if (alpha < 0.1)
        discard;
    
    // Grass colour (green tint)
    vec3 grassColor = vec3(0.3, 0.6, 0.2);
    
    // Lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    // Ambient
    float ambientStrength = 0.5;
    vec3 ambient = ambientStrength * grassColor;
    
    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * grassColor * 0.6;
    
    // Combine
    vec3 result = ambient + diffuse;
    
    FragColor = vec4(result, alpha);
}