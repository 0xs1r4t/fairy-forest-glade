// Colour Palette Library

// ===== GREENS (Foliage) =====
const vec3 DARK_FOREST_GREEN = vec3(0.1, 0.3, 0.15);
const vec3 DEEP_GREEN = vec3(0.2, 0.5, 0.2);
const vec3 MID_GREEN = vec3(0.4, 0.8, 0.3);
const vec3 BRIGHT_GREEN = vec3(0.5, 0.9, 0.4);
const vec3 LIME_GREEN = vec3(0.7, 1.0, 0.5);
const vec3 YELLOW_GREEN = vec3(0.8, 0.95, 0.5);

// Grass variations
const vec3 GRASS_DARK = vec3(0.15, 0.3, 0.1);
const vec3 GRASS_MID = vec3(0.3, 0.6, 0.2);
const vec3 GRASS_LIGHT = vec3(0.5, 0.8, 0.3);
const vec3 GRASS_TIP = vec3(0.7, 0.85, 0.4);

// ===== BROWNS (Wood/Bark) =====
const vec3 DARK_BARK = vec3(0.25, 0.15, 0.10);
const vec3 MID_BARK = vec3(0.40, 0.25, 0.15);
const vec3 LIGHT_BARK = vec3(0.55, 0.35, 0.20);
const vec3 HIGHLIGHT_BARK = vec3(0.7, 0.5, 0.3);

// ===== BLUES (Sky/Water) =====
const vec3 DEEP_BLUE = vec3(0.1, 0.2, 0.4);
const vec3 SKY_BLUE = vec3(0.3, 0.6, 0.9);
const vec3 LIGHT_SKY = vec3(0.6, 0.8, 1.0);

// ===== PURPLES/PINKS (Flowers) =====
const vec3 DEEP_PURPLE = vec3(0.3, 0.1, 0.4);
const vec3 PINK = vec3(0.9, 0.4, 0.7);
const vec3 LIGHT_PINK = vec3(1.0, 0.7, 0.9);

// ===== YELLOWS/ORANGES (Accents) =====
const vec3 GOLDEN = vec3(0.9, 0.7, 0.2);
const vec3 WARM_YELLOW = vec3(1.0, 0.9, 0.5);
const vec3 SUNSET_ORANGE = vec3(0.9, 0.5, 0.2);

// ===== NEUTRALS (Shadows/Highlights) =====
const vec3 DEEP_SHADOW = vec3(0.05, 0.05, 0.08);
const vec3 MID_SHADOW = vec3(0.2, 0.2, 0.25);
const vec3 HIGHLIGHT = vec3(0.95, 0.95, 1.0);

// Helper function: Cel-shading with custom color ramp
vec3 celShade3Band(float NdotL, vec3 darkColor, vec3 midColor, vec3 lightColor) {
    if (NdotL > 0.7) return lightColor;
    else if (NdotL > 0.3) return midColor;
    else return darkColor;
}

vec3 celShade4Band(float NdotL, vec3 shadowColor, vec3 darkColor, vec3 midColor, vec3 lightColor) {
    if (NdotL > 0.8) return lightColor;
    else if (NdotL > 0.5) return midColor;
    else if (NdotL > 0.2) return darkColor;
    else return shadowColor;
}

// Smooth cel-shading (posterized but less harsh)
vec3 celShadeSmooth(float NdotL, vec3 darkColor, vec3 lightColor, int bands) {
    float quantized = floor(NdotL * float(bands)) / float(bands);
    return mix(darkColor, lightColor, quantized);
}
