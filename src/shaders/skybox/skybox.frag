#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform sampler2D environmentMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    // Convert 3D direction to equirectangular UV coordinates
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
vec3 colour = texture(environmentMap, uv).rgb;
    
    // Tone mapping (simple Reinhard)
colour = colour / (colour + vec3(1.0));
    
    // Gamma correction
colour = pow(colour, vec3(1.0/2.2));
    
    FragColor = vec4(colour, 1.0);
}