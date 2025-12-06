#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aInstancePos;

out vec2 TexCoord;
out vec3 WorldPos;
out float HeightFactor;
out float WindInfluence;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;
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
    
    // Distance-based LOD scaling
    float distance = length(cameraPos - aInstancePos);
    float scale = 1.0;
    
    if (distance > 15.0) {
        scale = mix(1.0, 0.7, clamp((distance - 15.0) / 30.0, 0.0, 1.0));
    }
    
    // ===== WIND DISPLACEMENT =====
    float windSpeed = 1.2;    // Faster than trees
    float windStrength = 0.25; // Stronger sway
    
    // Sample noise at grass position + scrolling time
    vec2 windUV = aInstancePos.xz * 0.5 + time * windSpeed * vec2(0.6, 0.4);
    
    // Multiple noise layers for complexity
    float wind1 = noise(windUV) * 0.5 + 0.5;
    float wind2 = noise(windUV * 2.5 + time * 0.8) * 0.5 + 0.5;
    float wind3 = noise(windUV * 0.8 - time * 0.3) * 0.5 + 0.5; // Counter-wind
    
    float windNoise = (wind1 * 0.5 + wind2 * 0.3 + wind3 * 0.2) - 0.5;
    
    // Grass tips bend much more than base
    float heightInfluence = aPos.y * aPos.y; // Quadratic for more bend at top
    
    // Wind direction offset
    vec2 windDirection = vec2(
        windNoise * windStrength * heightInfluence,
        windNoise * windStrength * 0.6 * heightInfluence
    );
    
    // Add individual blade variation (prevents uniform wave look)
    float bladeVariation = fract(sin(dot(aInstancePos.xz, vec2(12.9898, 78.233))) * 43758.5453);
    windDirection *= 0.8 + bladeVariation * 0.4;
    
    // Apply wind offset to instance position
    vec3 windOffset = vec3(windDirection.x, 0.0, windDirection.y);
    vec3 instancePosWithWind = aInstancePos + windOffset;
    
    // Camera-facing billboard (cylindrical)
    vec3 toCamera = normalize(cameraPos - instancePosWithWind);
    toCamera.y = 0.0;
    toCamera = normalize(toCamera);
    
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, toCamera));
    
    // Apply scaled billboard
    vec3 worldPos = instancePosWithWind + right * (aPos.x * scale) + up * (aPos.y * scale);
    WorldPos = worldPos;
    HeightFactor = aPos.y;
    WindInfluence = (wind1 + wind2) * 0.5;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
}
