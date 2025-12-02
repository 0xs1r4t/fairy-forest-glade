#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <iostream>
using namespace std;

class ShaderLibrary
{
public:
    // Load npr_colors.glsl content
    static string GetColours()
    {
        ifstream file("src/shaders/colours.glsl");
        if (!file.is_open())
        {
            cerr << "Failed to load colour library!" << endl;
            return "";
        }
        stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    // Load shader with automatic library injection
    static string LoadShaderWithLibrary(const char *path)
    {
        ifstream file(path);
        if (!file.is_open())
        {
            cerr << "Failed to load shader: " << path << endl;
            return "";
        }

        stringstream buffer;
        string line;
        bool versionFound = false;

        while (getline(file, line))
        {
            buffer << line << "\n";

            // Inject library after #version directive
            if (!versionFound && line.find("#version") != string::npos)
            {
                buffer << "\n// === NPR Color Library ===\n";
                buffer << GetColours() << "\n";
                buffer << "// === End Library ===\n\n";
                versionFound = true;
            }
        }

        return buffer.str();
    }
};
