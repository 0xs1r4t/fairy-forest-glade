#pragma once

#include <glm/glm.hpp>
#include <cmath>

// Perlin-style noise implementation
class Noise
{
public:
    // Hash function for pseudo-random values
    static float hash(float n)
    {
        return glm::fract(sin(n) * 43758.5453123f);
    }

    // 2D hash
    static float hash2D(glm::vec2 p)
    {
        return glm::fract(sin(glm::dot(p, glm::vec2(12.9898f, 78.233f))) * 43758.5453123f);
    }

    // Simple 2D noise (value noise)
    static float noise2D(glm::vec2 p)
    {
        glm::vec2 i = glm::floor(p);
        glm::vec2 f = glm::fract(p);

        // Smooth interpolation (smoothstep)
        f = f * f * (3.0f - 2.0f * f);

        // Four corners of the grid cell
        float a = hash2D(i);
        float b = hash2D(i + glm::vec2(1.0f, 0.0f));
        float c = hash2D(i + glm::vec2(0.0f, 1.0f));
        float d = hash2D(i + glm::vec2(1.0f, 1.0f));

        // Bilinear interpolation
        return glm::mix(glm::mix(a, b, f.x), glm::mix(c, d, f.x), f.y);
    }

    // Fractional Brownian Motion (FBM)
    static float fbm(glm::vec2 p, int octaves = 6, float lacunarity = 2.0f, float gain = 0.5f)
    {
        float amplitude = 0.5f;
        float frequency = 1.0f;
        float value = 0.0f;
        float maxValue = 0.0f;

        for (int i = 0; i < octaves; i++)
        {
            value += amplitude * noise2D(p * frequency);
            maxValue += amplitude;

            frequency *= lacunarity;
            amplitude *= gain;
        }

        return value / maxValue; // Normalize to [0, 1]
    }

    // Domain warping FBM (more interesting terrain)
    static float warpedFBM(glm::vec2 p, int octaves = 6)
    {
        glm::vec2 q = glm::vec2(fbm(p, octaves), fbm(p + glm::vec2(5.2f, 1.3f), octaves));
        glm::vec2 r = glm::vec2(fbm(p + 4.0f * q + glm::vec2(1.7f, 9.2f), octaves),
                                fbm(p + 4.0f * q + glm::vec2(8.3f, 2.8f), octaves));
        return fbm(p + 4.0f * r, octaves);
    }

    // Ridged noise (good for mountains)
    static float ridgedNoise(glm::vec2 p, int octaves = 6)
    {
        float amplitude = 0.5f;
        float frequency = 1.0f;
        float value = 0.0f;
        float weight = 1.0f;

        for (int i = 0; i < octaves; i++)
        {
            float n = noise2D(p * frequency);
            n = 1.0f - abs(n * 2.0f - 1.0f); // Create ridges
            n = n * n * weight;              // Square for sharper ridges

            weight = glm::clamp(n * 2.0f, 0.0f, 1.0f);
            value += n * amplitude;

            frequency *= 2.0f;
            amplitude *= 0.5f;
        }

        return value;
    }
};