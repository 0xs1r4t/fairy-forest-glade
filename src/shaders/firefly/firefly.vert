#version 330 core
layout (location = 0) in vec3 aPos;       // Quad vertex position
layout (location = 1) in vec3 aOffset;    // Instance position (world space)
layout (location = 2) in vec3 aColor;     // Instance color
layout (location = 3) in float aSize;     // Instance size

uniform mat4 view;
uniform mat4 projection;
uniform float time;

out vec3 Color;
out float Glow;
out vec2 TexCoord;

void main() {
    // Pulsing glow effect
    float pulse = sin(time * 2.0 + aOffset.x * 5.0 + aOffset.z * 3.0) * 0.5 + 0.5;
    Glow = 0.7 + pulse * 0.3;
    
    // Extract camera right and up vectors from view matrix (for billboarding)
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    
    // Build billboard in WORLD SPACE (not view space!)
    vec3 worldPos = aOffset 
                  + cameraRight * aPos.x * aSize * (1.0 + pulse * 0.3)
                  + cameraUp * aPos.y * aSize * (1.0 + pulse * 0.3);
    
    // Now apply view and projection normally
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    Color = aColor;
    TexCoord = aPos.xy + 0.5;
}
