#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;   // For normal mapping

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform float animationPhase;
uniform bool leafBillboarding;
uniform vec3 viewPos;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;

// Simple wind animation for leaves
vec3 applyWindAnimation(vec3 pos, vec3 normal) {
    // Only affect vertices high up (leaves, not trunk)
    float heightFactor = smoothstep(0.0, 5.0, pos.y);
    
    // Gentle swaying
    float wind = sin(time * 2.0 + animationPhase + pos.x * 0.5) * 0.1;
    wind += sin(time * 3.0 + animationPhase + pos.z * 0.3) * 0.05;
    
    pos.x += wind * heightFactor;
    pos.z += wind * 0.5 * heightFactor;
    
    // Leaf flutter
    float flutter = sin(time * 8.0 + animationPhase) * 0.02;
    pos.y += flutter * heightFactor * heightFactor; // Only tips flutter
    
    return pos;
}

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    
    // Apply wind animation
    worldPos.xyz = applyWindAnimation(worldPos.xyz, aNormal);
    
    FragPos = worldPos.xyz;
    TexCoords = aTexCoords;
    
    // Normal mapping setup (TBN matrix)
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    
    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    TBN = mat3(T, B, N);
    Normal = N;
    
    // Leaf billboarding (optional - for flat leaf planes)
    if (leafBillboarding) {
        // Make leaves face camera
        vec3 toCamera = normalize(viewPos - FragPos);
        toCamera.y *= 0.3; // Reduce vertical tilt
        Normal = normalize(toCamera);
    }
    
    gl_Position = projection * view * worldPos;
}