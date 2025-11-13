#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 VertexColor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 materialColor;
uniform float materialShininess;
uniform bool useVertexColor;

// Add environment map
uniform sampler2D environmentMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec3 finalMaterialColor = useVertexColor ? VertexColor : materialColor;
    float finalShininess = materialShininess;

    // Basic lighting
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), finalShininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 envColor = texture(environmentMap, SampleSphericalMap(norm)).rgb;
    vec3 envAmbient = 0.3 * envColor;

    vec3 R = reflect(-viewDir, norm);
    vec3 envReflection = texture(environmentMap, SampleSphericalMap(R)).rgb;

    float reflectivity = finalShininess / 128.0;

    vec3 result = (envAmbient + diffuse) * finalMaterialColor +
                  specular * lightColor +
                  reflectivity * envReflection;

    FragColor = vec4(result, 1.0);
}