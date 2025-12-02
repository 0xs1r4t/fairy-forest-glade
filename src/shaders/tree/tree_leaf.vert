#version 330 core
layout (location = 0) in vec2 aQuadPos;   // Quad vertex (-0.5 to 0.5)
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aWorldPos;      // Per-instance: leaf world position
layout (location = 3) in vec3 aCustomNormal;  // Per-instance: for lighting gradient
layout (location = 4) in float aScale;        // Per-instance: size
layout (location = 5) in float aTexIndex;     // Per-instance: which texture

out vec2 TexCoord;
out vec3 CustomNormal;
out vec3 WorldPos;
flat out int TexIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraRight;  // Camera's right vector (for billboarding)
uniform vec3 cameraUp;     // Camera's up vector

void main() {
    TexCoord = aTexCoord;
    CustomNormal = mat3(model) * aCustomNormal; // Transform to world space
    TexIndex = int(aTexIndex);
    
    // Billboard: construct quad facing camera
    vec3 worldCenter = (model * vec4(aWorldPos, 1.0)).xyz;
    vec3 vertexOffset = (aQuadPos.x * cameraRight + aQuadPos.y * cameraUp) * aScale;
    WorldPos = worldCenter + vertexOffset;
    
    gl_Position = projection * view * vec4(WorldPos, 1.0);
}
