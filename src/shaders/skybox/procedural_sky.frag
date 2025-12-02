#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform float time;
uniform vec3 moonDir;

// Simple hash for stars
float hash(vec3 p) {
    p = fract(p * vec3(443.537, 537.247, 247.428));
    p += dot(p, p.yxz + 19.19);
    return fract((p.x + p.y) * p.z);
}

// Improved noise for stars
float noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);
    
    float n = p.x + p.y * 157.0 + 113.0 * p.z;
    return mix(
        mix(mix(hash(p + vec3(0,0,0)), hash(p + vec3(1,0,0)), f.x),
            mix(hash(p + vec3(0,1,0)), hash(p + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash(p + vec3(0,0,1)), hash(p + vec3(1,0,1)), f.x),
            mix(hash(p + vec3(0,1,1)), hash(p + vec3(1,1,1)), f.x), f.y), f.z);
}

// Generate stars
float stars(vec3 rd) {
    // Only render stars above horizon
    if (rd.y < 0.0) return 0.0;
    
    // Scale up the sky dome
    vec3 p = rd * 400.0;
    vec3 c = floor(p);
    
    // Random star at each grid cell
    float h = hash(c);
    
    // Star threshold (lower = more stars)
    float star = step(0.999, h);
    
    // Star brightness variation
    float brightness = hash(c * 1.234) * 0.5 + 0.5;
    
    // Twinkling
    brightness *= 0.8 + 0.2 * sin(time * 3.0 + h * 100.0);
    
    return star * brightness * 0.5; // Adjust final brightness
}

// Sky gradient
vec3 skyGradient(vec3 rd) {
    // Colors
    vec3 zenithColor = vec3(0.05, 0.05, 0.15);      // Dark blue at top
    vec3 horizonColor = vec3(0.15, 0.10, 0.25);     // Purple at horizon
    vec3 nadirColor = vec3(0.02, 0.02, 0.08);       // Very dark below
    
    // Smooth gradient based on Y direction
    vec3 sky = mix(nadirColor, horizonColor, smoothstep(-0.2, 0.1, rd.y));
    sky = mix(sky, zenithColor, smoothstep(0.1, 0.8, rd.y));
    
    return sky;
}

// Moon rendering (FIXED - no black hole)
vec3 renderMoon(vec3 rd, vec3 moonDir) {
    vec3 col = vec3(0.0);
    
    // Moon properties
    float moonSize = 0.035;
    vec3 moonColor = vec3(1.0, 0.95, 0.85);
    
    // Distance from ray to moon center
    float dist = length(rd - moonDir);
    
    // Moon disc (FIXED - better edge)
    float moon = smoothstep(moonSize * 1.05, moonSize * 0.95, dist);
    
    if (moon > 0.01) {
        // Simple shading on moon surface
        vec3 toMoon = normalize(rd);
        vec3 moonNormal = normalize(toMoon - moonDir * dot(toMoon, moonDir));
        vec3 lightDir = normalize(vec3(-moonDir.x, 0.3, -moonDir.z));
        
        float moonShading = dot(moonNormal, lightDir) * 0.5 + 0.5;
        moonShading = pow(moonShading, 0.7);
        
        col += moon * moonColor * (0.5 + 0.5 * moonShading);
    }
    
    // Moon glow (FIXED - limited range)
    float glow = exp(-dist * 12.0) * 0.25;
    glow = max(glow, 0.0);
    col += glow * moonColor * 0.3;
    
    return col;
}

void main() {
    vec3 rd = normalize(TexCoords);
    
    // Base sky gradient
    vec3 col = skyGradient(rd);
    
    // Add stars with MUCH HIGHER brightness
    float starField = stars(rd);
    col += vec3(starField * 15.0, starField * 14.0, starField * 16.0); // MASSIVELY INCREASED
    
    // Add moon
    vec3 moonLight = renderMoon(rd, moonDir);
    col += moonLight;
    
    // Horizon glow
    float horizonGlow = exp(-max(abs(rd.y), 0.0) * 4.0) * 0.06;
    col += vec3(0.15, 0.12, 0.25) * horizonGlow * smoothstep(-0.1, 0.1, rd.y);
    
    // Exposure
    col = pow(col, vec3(0.9));
    col = clamp(col, 0.0, 1.0);
    
    FragColor = vec4(col, 1.0);
}