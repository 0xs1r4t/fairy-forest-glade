#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
using namespace std;

class TextureGenerator {
public:
    // Generate grass texture with green gradient and alpha
    static vector<unsigned char> GenerateGrassTexture(int width, int height) {
        vector<unsigned char> data(width * height * 4); // RGBA
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;
                
                // Create grass blade shape
                float centerX = width / 2.0f;
                float distFromCenter = abs((float)x - centerX);
                
                // Height factor: 0 at bottom, 1 at top
                float heightFactor = y / (float)height;
                
                // Taper width from bottom to top
                float taperFactor = 1.0f - heightFactor * 0.7f;
                float bladeWidth = (width * 0.2f) * taperFactor;
                
                // Create multiple blades for variety
                bool blade1 = distFromCenter < bladeWidth * 0.4f;
                bool blade2 = abs((float)x - width * 0.3f) < bladeWidth * 0.3f;
                bool blade3 = abs((float)x - width * 0.7f) < bladeWidth * 0.3f;
                
                bool isGrass = blade1 || blade2 || blade3;
                
                if (isGrass) {
                    // Green gradient: dark at bottom, light at top
                    // Bottom color: RGB(45, 80, 22) - dark green
                    // Top color: RGB(157, 255, 112) - light green
                    unsigned char r = (unsigned char)(45 + heightFactor * 112);
                    unsigned char g = (unsigned char)(80 + heightFactor * 175);
                    unsigned char b = (unsigned char)(22 + heightFactor * 90);
                    
                    // Alpha: solid except fade at very top
                    unsigned char a = 255;
                    if (heightFactor > 0.85f) {
                        float fadeStart = (heightFactor - 0.85f) / 0.15f;
                        a = (unsigned char)(255 * (1.0f - fadeStart));
                    }
                    
                    data[idx + 0] = r; // Red
                    data[idx + 1] = g; // Green
                    data[idx + 2] = b; // Blue
                    data[idx + 3] = a; // Alpha
                } else {
                    // Fully transparent (black areas)
                    data[idx + 0] = 0;
                    data[idx + 1] = 0;
                    data[idx + 2] = 0;
                    data[idx + 3] = 0; // Alpha = 0 (transparent)
                }
            }
        }
        
        return data;
    }
    
    // Generate flower texture with petals and stem
    static vector<unsigned char> GenerateFlowerTexture(int width, int height) {
        vector<unsigned char> data(width * height * 4);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;
                
                float centerX = width / 2.0f;
                float centerY = height * 0.75f; // Flower center at top
                
                // Distance from flower center
                float dx = x - centerX;
                float dy = y - centerY;
                float distFromFlowerCenter = sqrt(dx * dx + dy * dy);
                
                // Stem parameters
                float stemWidth = width * 0.06f;
                float distFromStemCenter = abs((float)x - centerX);
                
                // Check if this pixel is part of flower (top half)
                bool isPetal = (y > height * 0.5f && distFromFlowerCenter < width * 0.25f);
                
                // Check if this pixel is part of stem (bottom half)
                bool isStem = (y <= height * 0.5f && distFromStemCenter < stemWidth);
                
                if (isPetal) {
                    // Pink/purple petals
                    // Create petal pattern with some variation
                    float angle = atan2(dy, dx);
                    float petalPattern = sin(angle * 5.0f) * 0.5f + 0.5f; // 5 petals
                    
                    // Color varies from center to edge
                    float colorFactor = 1.0f - (distFromFlowerCenter / (width * 0.25f));
                    
                    data[idx + 0] = (unsigned char)(200 + petalPattern * 55);  // Red
                    data[idx + 1] = (unsigned char)(80 + colorFactor * 100);   // Green
                    data[idx + 2] = (unsigned char)(150 + petalPattern * 50);  // Blue
                    data[idx + 3] = 255; // Solid
                    
                } else if (isStem) {
                    // Green stem - darker than grass
                    float heightFactor = y / (float)height;
                    
                    data[idx + 0] = (unsigned char)(30 + heightFactor * 20);  // Red
                    data[idx + 1] = (unsigned char)(100 + heightFactor * 40); // Green
                    data[idx + 2] = (unsigned char)(30 + heightFactor * 10);  // Blue
                    data[idx + 3] = 255; // Solid
                    
                } else {
                    // Transparent
                    data[idx + 0] = 0;
                    data[idx + 1] = 0;
                    data[idx + 2] = 0;
                    data[idx + 3] = 0;
                }
            }
        }
        
        return data;
    }
    
    // Generate tree texture with trunk and foliage
    static vector<unsigned char> GenerateTreeTexture(int width, int height) {
        vector<unsigned char> data(width * height * 4);
        
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;
                
                float centerX = width / 2.0f;
                float heightFactor = y / (float)height;
                float distFromCenter = abs((float)x - centerX);
                
                // Trunk: bottom 40% of texture, thin vertical strip
                bool isTrunk = (heightFactor < 0.4f && 
                               distFromCenter < width * 0.08f);
                
                // Foliage: top 70% (overlaps with trunk), wider oval shape
                // Tapers at top
                float foliageWidth = width * 0.35f;
                if (heightFactor > 0.7f) {
                    // Taper near top
                    foliageWidth *= (1.0f - (heightFactor - 0.7f) / 0.3f) * 0.5f + 0.5f;
                }
                
                bool isFoliage = (heightFactor >= 0.3f && 
                                 distFromCenter < foliageWidth);
                
                if (isTrunk) {
                    // Brown trunk with some texture variation
                    int variation = (x + y) % 20 - 10; // -10 to +10
                    
                    data[idx + 0] = (unsigned char)(101 + variation); // Red
                    data[idx + 1] = (unsigned char)(67 + variation);  // Green
                    data[idx + 2] = (unsigned char)(33 + variation);  // Blue
                    data[idx + 3] = 255; // Solid
                    
                } else if (isFoliage) {
                    // Green foliage - darker toward center, lighter at edges
                    float edgeFactor = distFromCenter / foliageWidth;
                    int variation = (x * 7 + y * 11) % 40 - 20; // Random variation
                    
                    data[idx + 0] = (unsigned char)(34 + variation + edgeFactor * 30);  // Red
                    data[idx + 1] = (unsigned char)(139 + variation - edgeFactor * 20); // Green
                    data[idx + 2] = (unsigned char)(34 + variation);                    // Blue
                    
                    // Slightly transparent at edges for depth
                    if (edgeFactor > 0.8f) {
                        data[idx + 3] = (unsigned char)(255 * (1.0f - (edgeFactor - 0.8f) / 0.2f));
                    } else {
                        data[idx + 3] = 220;
                    }
                    
                } else {
                    // Transparent
                    data[idx + 0] = 0;
                    data[idx + 1] = 0;
                    data[idx + 2] = 0;
                    data[idx + 3] = 0;
                }
            }
        }
        
        return data;
    }
    
    // Helper: Generate a simple solid color texture
    static vector<unsigned char> GenerateSolidColor(int width, int height, 
                                                          unsigned char r, 
                                                          unsigned char g, 
                                                          unsigned char b, 
                                                          unsigned char a = 255) {
        vector<unsigned char> data(width * height * 4);
        
        for (int i = 0; i < width * height; i++) {
            data[i * 4 + 0] = r;
            data[i * 4 + 1] = g;
            data[i * 4 + 2] = b;
            data[i * 4 + 3] = a;
        }
        
        return data;
    }
    
    // Helper: Add noise to a texture
    static void AddNoise(vector<unsigned char>& data, int width, int height, float intensity) {
        for (int i = 0; i < width * height; i++) {
            // Only add noise to non-transparent pixels
            if (data[i * 4 + 3] > 0) {
                int noise = (rand() % 40) - 20; // -20 to +20
                
                for (int c = 0; c < 3; c++) { // RGB only, not alpha
                    int newVal = data[i * 4 + c] + (int)(noise * intensity);
                    data[i * 4 + c] = (unsigned char)max(0, min(255, newVal));
                }
            }
        }
    }
};