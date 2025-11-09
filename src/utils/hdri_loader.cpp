#include <GL/glew.h>
#include <Imath/ImathBox.h>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>

#include <iostream>
using namespace std;

#include "hdri_loader.h"

// Load an EXR file and return OpenGL texture ID
unsigned int loadHDR(const char *path, int &width, int &height)
{
    try
    {
        // Open the EXR file
        Imf::RgbaInputFile file(path);
        Imath::Box2i dw = file.dataWindow();

        // Get dimensions
        width = dw.max.x - dw.min.x + 1;
        height = dw.max.y - dw.min.y + 1;

        // Allocate memory for pixel data
        Imf::Array2D<Imf::Rgba> pixels(height, width);

        // Read the pixels
        file.setFrameBuffer(&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
        file.readPixels(dw.min.y, dw.max.y);

        // Convert to float array for OpenGL (RGB only, 3 channels)
        float *data = new float[width * height * 3];
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                int flippedY = height - 1 - y; // Flip vertically for OpenGL
                int idx = (y * width + x) * 3;
                data[idx + 0] = pixels[flippedY][x].r;
                data[idx + 1] = pixels[flippedY][x].g;
                data[idx + 2] = pixels[flippedY][x].b;
                // Note: Alpha channel ignored for HDR environment maps
            }
        }

        // Create OpenGL texture
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        delete[] data;

        std::cout << "HDR texture loaded: " << width << "x" << height << std::endl;
        return textureID;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error loading EXR file: " << e.what() << std::endl;
        return 0;
    }
}