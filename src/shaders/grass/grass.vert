#version 330 core
layout (location = 0) in vec3 aPos;      // Quad vertex position
layout (location = 1) in vec3 aNormal;   // Not used for billboards
layout (location = 2) in vec2 aTexCoord; 
layout (location = 3) in vec3 aInstancePos; // Per-instance world position

out vec2 TexCoord;
out vec3 WorldPos;
out float HeightFactor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main() {
    TexCoord = aTexCoord;
    
    // Calculate camera-facing vectors (cylindrical billboarding - only rotate around Y)
    vec3 toCamera = normalize(cameraPos - aInstancePos);
    toCamera.y = 0.0; // Keep upright
    toCamera = normalize(toCamera);
    
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, toCamera));
    
    // Billboard the quad (grass blade)
    // aPos.x is horizontal offset, aPos.y is height
    vec3 worldPos = aInstancePos + right * aPos.x + up * aPos.y;
    WorldPos = worldPos;
    
    // Height factor for color variation
    HeightFactor = aPos.y;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
