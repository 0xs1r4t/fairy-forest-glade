#version 330 core
layout (location = 0) in vec2 aQuadPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aWorldPos;
layout (location = 3) in vec3 aCustomNormal;
layout (location = 4) in float aScale;
layout (location = 5) in float aTexIndex;

out vec2 TexCoord;
out vec3 CustomNormal;
out vec3 WorldPos;
out float WindInfluence;
flat out int TexIndex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform float time;

// Wind noise functions
vec2 hash(vec2 p) {
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float noise(in vec2 p) {
    const float K1 = 0.366025404;
    const float K2 = 0.211324865;
    vec2 i = floor(p + (p.x + p.y) * K1);
    vec2 a = p - i + (i.x + i.y) * K2;
    vec2 o = (a.x > a.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec2 b = a - o + K2;
    vec2 c = a - 1.0 + 2.0 * K2;
    vec3 h = max(0.5 - vec3(dot(a, a), dot(b, b), dot(c, c)), 0.0);
    vec3 n = h * h * h * h * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
    return dot(n, vec3(70.0));
}

void main() {
    TexCoord = aTexCoord;
    TexIndex = int(aTexIndex);
    
    // Transform instance position to world space
    vec3 worldCenter = (model * vec4(aWorldPos, 1.0)).xyz;
    
    // ===== WIND EFFECT =====
    float windSpeed = 0.7;
    float windStrength = 0.12;
    
    // Sample scrolling noise based on world position
    vec2 windUV = worldCenter.xz * 0.3 + time * windSpeed * vec2(0.5, 0.3);
    
    // Layer multiple noise octaves for natural movement
    float wind1 = noise(windUV) * 0.5 + 0.5;
    float wind2 = noise(windUV * 2.3 + time * 0.4) * 0.5 + 0.5;
    float windNoise = (wind1 + wind2 * 0.5) - 0.75; // Center around 0
    
    // Higher leaves sway more
    float heightFactor = smoothstep(0.0, 3.0, worldCenter.y);
    
    // Apply wind offset to the center position
    vec3 windOffset = vec3(
        windNoise * windStrength * heightFactor,
        windNoise * windStrength * 0.2 * heightFactor, // Less vertical
        windNoise * windStrength * 0.7 * heightFactor
    );
    
    worldCenter += windOffset;
    
    // Billboard facing camera
    vec3 vertexOffset = (aQuadPos.x * cameraRight + aQuadPos.y * cameraUp) * aScale;
    WorldPos = worldCenter + vertexOffset;
    
    gl_Position = projection * view * vec4(WorldPos, 1.0);
    
    // Pass wind influence for fragment shader shimmer
    WindInfluence = (wind1 + wind2) * 0.5 * heightFactor;
    
    // Transform normal to world space
    CustomNormal = mat3(model) * aCustomNormal;
}
