#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform float time;
uniform vec3 moonDir;

// ===== STAR FUNCTIONS =====

float hash3(vec3 p) {
    p = fract(p * vec3(443.537, 537.247, 247.428));
    p += dot(p, p.yxz + 19.19);
    return fract((p.x + p.y) * p.z);
}

float stars(vec3 rd) {
    if (rd.y < 0.0) return 0.0;
    
    vec3 p = rd * 500.0;
    vec3 c = floor(p);
    
    float h = hash3(c);
    float star = step(0.999, h); // star threshold
    float brightness = hash3(c * 1.234) * 0.5 + 0.5;
    brightness *= 0.8 + 0.2 * sin(time * 3.0 + h * 100.0); // Twinkle
    
    return star * brightness * 0.5;
}

// ===== SKY GRADIENT =====

vec3 skyGradient(vec3 rd) {
    vec3 zenithColor = vec3(0.05, 0.05, 0.15);
    vec3 horizonColor = vec3(0.15, 0.10, 0.25);
    vec3 nadirColor = vec3(0.02, 0.02, 0.08);
    
    vec3 sky = mix(nadirColor, horizonColor, smoothstep(-0.2, 0.1, rd.y));
    sky = mix(sky, zenithColor, smoothstep(0.1, 0.8, rd.y));
    
    return sky;
}

// ===== SMOOTH MOON =====

vec3 renderMoon(vec3 rd, vec3 moonDir) {
    vec3 col = vec3(0.0);
    
    float moonSize = 0.035;
    vec3 moonColor = vec3(1.0, 0.95, 0.85);
    
    // Distance from ray to moon center
    float dist = length(rd - moonDir);
    
    // Smooth moon disc
    float moon = smoothstep(moonSize * 1.05, moonSize * 0.95, dist);
    
    if (moon > 0.01) {
        // Calculate position on moon surface (0 at center, 1 at edge)
        float edgeDist = dist / moonSize;
        
        // Smooth radial gradient from center (bright) to edge (darker)
        float brightness = 1.0 - smoothstep(0.0, 1.0, edgeDist);
        brightness = pow(brightness, 0.6); // Adjust falloff curve
        
        // Blend between full brightness at center and darker at edges
        float shading = mix(0.6, 1.0, brightness); // 0.6 = edge darkness, 1.0 = center brightness
        
        col += moon * moonColor * shading;
    }
    
    // Soft outer glow
    float glow = exp(-dist * 12.0) * 0.25;
    col += glow * moonColor * 0.3;
    
    return col;
}

// ===== MAIN =====

void main() {
    vec3 rd = normalize(TexCoords);
    
    // Base sky gradient
    vec3 col = skyGradient(rd);
    
    // Add stars
    float starField = stars(rd);
    col += vec3(starField * 2.5, starField * 2.2, starField * 2.8);
    
    // Add moon
    vec3 moonLight = renderMoon(rd, moonDir);
    col += moonLight;
    
    // Horizon glow
    float horizonGlow = exp(-max(abs(rd.y), 0.0) * 4.0) * 0.06;
    col += vec3(0.15, 0.12, 0.25) * horizonGlow;
    
    // Exposure
    col = pow(col, vec3(0.9));
    col = clamp(col, 0.0, 1.0);
    
    FragColor = vec4(col, 1.0);
}
