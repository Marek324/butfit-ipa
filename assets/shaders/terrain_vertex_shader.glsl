#version 330 core
// Vertex Shader for Terrain Rendering

layout (location = 0) in vec3 aPos;      // Vertex position
layout (location = 1) in vec3 aNormal;   // Vertex normal
layout (location = 2) in vec2 aTexCoord; // Texture coordinates

out vec2 TexCoord;
out vec3 FragPosWorld;
out vec3 NormalWorld;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main() {
    gl_Position = projection * view  * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    FragPosWorld = vec3(model * vec4(aPos, 1.0));
    NormalWorld = normalize(normalMatrix * aNormal);
}