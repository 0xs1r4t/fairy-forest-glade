#pragma once

// Load an EXR file and return OpenGL texture ID
unsigned int loadHDR(const char *path, int &width, int &height);
