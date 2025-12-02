#version 330 core
layout (location = 0) in vec3 aPos;      
layout (location = 1) in vec3 aNormal;   
layout (location = 2) in vec2 aTexCoord; 
layout (location = 3) in vec3 aInstancePos;

out vec2 TexCoord;
out vec3 WorldPos;
out float HeightFactor;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main() {
    TexCoord = aTexCoord;
    
    // Distance-based LOD scaling
    float distance = length(cameraPos - aInstancePos);
    float scale = 1.0;
    
    if (distance > 15.0) {
        // Slightly scale down distant grass (15m+)
        scale = mix(1.0, 0.7, clamp((distance - 15.0) / 30.0, 0.0, 1.0));
    }
    
    // Camera-facing billboard (cylindrical)
    vec3 toCamera = normalize(cameraPos - aInstancePos);
    toCamera.y = 0.0;
    toCamera = normalize(toCamera);
    
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, toCamera));
    
    // Apply scaled billboard
    vec3 worldPos = aInstancePos + right * (aPos.x * scale) + up * (aPos.y * scale);
    WorldPos = worldPos;
    HeightFactor = aPos.y;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
