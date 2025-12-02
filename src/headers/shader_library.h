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
    static string GetNPRColors()
    {
        ifstream file("src/shaders/colours.glsl");
        if (!file.is_open())
        {
            cerr << "ERROR: Failed to load NPR color library at src/shaders/colours.glsl!" << endl;
            return "// NPR library missing\n";
        }
        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();

        // DEBUG OUTPUT
        cout << "NPR Library loaded: " << content.length() << " characters" << endl;

        return content;
    }

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
                buffer << "\n// === NPR Color Library Injected ===\n";
                buffer << GetNPRColors();
                buffer << "\n// === End NPR Library ===\n\n";
                versionFound = true;
            }
        }

        if (!versionFound)
        {
            cerr << "WARNING: No #version directive found in " << path << endl;
        }

        return buffer.str();
    }
};
