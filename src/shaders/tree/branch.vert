#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

// Same noise functions
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
    vec4 worldPos = model * vec4(aPos, 1.0);
    
    // Gentle branch sway (much less than leaves)
    float windSpeed = 0.5;
    float windStrength = 0.04; // Very subtle
    
    vec2 windUV = worldPos.xz * 0.15 + time * windSpeed * vec2(0.5, 0.3);
    float windNoise = noise(windUV) * 0.5 + 0.5;
    windNoise = (windNoise - 0.5) * 2.0; // Center around 0
    
    // Only upper parts sway
    float heightFactor = smoothstep(0.0, 2.5, aPos.y);
    
    // Apply subtle sway
    vec3 windOffset = vec3(
        windNoise * windStrength * heightFactor,
        0.0, // No vertical for branches
        windNoise * windStrength * 0.6 * heightFactor
    );
    
    worldPos.xyz += windOffset;
    
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    
    gl_Position = projection * view * worldPos;
}
