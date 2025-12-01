#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 instanceOffset;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 fairyPos;
uniform float fairyRadius;
uniform vec3 viewPos;

out vec2 TexCoords;
out vec3 Normal;
out vec3 FragPos;

void main() {
    // Get camera right and up vectors from view matrix
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp = vec3(0.0, 1.0, 0.0); // Keep upright (cylindrical billboard)
    
    // For perfect spherical billboarding (faces camera even when looking down):
    // vec3 cameraUp = vec3(view[0][1], view[1][1], view[2][1]);
    
    // Flora interaction - scale up near fairy
    float dist = length(vec2(fairyPos.x - instanceOffset.x, fairyPos.z - instanceOffset.z));
    float influence = smoothstep(fairyRadius, 0.0, dist);
    float extraScale = 1.0 + influence * 0.3;
    
    // Reconstruct vertex position to face camera
    // aPos.x and aPos.z determine offset from center
    // aPos.y is the height
    vec3 billboardPos = instanceOffset + 
                        cameraRight * aPos.x + 
                        cameraUp * aPos.y * extraScale; // Apply scaling to height
    
    FragPos = billboardPos;
    TexCoords = aTexCoords;
    
    // Normal faces camera
    vec3 toCamera = viewPos - instanceOffset;
    toCamera.y = 0.0; // Cylindrical
    Normal = normalize(toCamera);
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}