#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 instanceOffset;
layout (location = 4) in float textureIndex;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 fairyPos;
uniform float fairyRadius;
uniform vec3 viewPos;
uniform float time; // NEW - for wind animation

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;
flat out int TexIndex;
out float HeightFactor; // NEW - for wind
out float WindInfluence; // NEW - for shader effects

// Wind noise functions (same as grass)
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
    vec3 h = max(0.5 - vec3(dot(a,a), dot(b,b), dot(c,c)), 0.0);
    vec3 n = h * h * h * h * vec3(dot(a, hash(i + 0.0)), dot(b, hash(i + o)), dot(c, hash(i + 1.0)));
    return dot(n, vec3(70.0));
}

void main() {
    // Extract camera RIGHT and UP vectors for billboarding
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(0.0, 1.0, 0.0);
    
    // ===== WIND DISPLACEMENT =====
    float windSpeed = 0.8; // Slower than grass (flowers are sturdier)
    float windStrength = 0.15; // Less sway than grass
    
    // Sample noise at flower position + scrolling time
    vec2 windUV = instanceOffset.xz * 0.5 + time * windSpeed * vec2(0.6, 0.4);
    
    // Multiple noise layers for complexity
    float wind1 = noise(windUV) * 0.5 + 0.5;
    float wind2 = noise(windUV * 2.5 + time * 0.8) * 0.5 + 0.5;
    float wind3 = noise(windUV * 0.8 - time * 0.3) * 0.5 + 0.5;
    
    float windNoise = (wind1 * 0.5 + wind2 * 0.3 + wind3 * 0.2) - 0.5;
    
    // Height influence - only top of flower sways
    float heightInfluence = aPos.y * aPos.y; // Quadratic for more bend at top
    
    // Wind direction offset
    vec2 windDirection = vec2(windNoise * windStrength * heightInfluence, 
                              windNoise * windStrength * 0.6 * heightInfluence);
    
    // Add individual flower variation
    float flowerVariation = fract(sin(dot(instanceOffset.xz, vec2(12.9898, 78.233))) * 43758.5453);
    windDirection *= (0.8 + flowerVariation * 0.4);
    
    // Apply wind offset to instance position
    vec3 windOffset = vec3(windDirection.x, 0.0, windDirection.y);
    vec3 instancePosWithWind = instanceOffset + windOffset;
    
    // Flora interaction - scale up near fairy
    float dist = length(vec2(fairyPos.x - instanceOffset.x, fairyPos.z - instanceOffset.z));
    float influence = smoothstep(fairyRadius, 0.0, dist);
    float extraScale = 1.0 + influence * 0.3;
    
    // Billboard position facing camera
    vec3 billboardPos = instancePosWithWind 
                      + cameraRight * aPos.x 
                      + cameraUp * aPos.y * extraScale;
    
    FragPos = billboardPos;
    TexCoords = aTexCoords;
    TexIndex = int(textureIndex);
    HeightFactor = aPos.y; // Pass to fragment shader
    WindInfluence = (wind1 + wind2) * 0.5; // Pass wind strength
    
    // Normal faces camera (cylindrical)
    vec3 toCamera = viewPos - instanceOffset;
    toCamera.y = 0.0;
    Normal = normalize(toCamera);
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
