#version 330 core
// Vertex Shader for Ocean Rendering

// Input vertex attributes (from VBOs)
layout (location = 0) in vec3 aPos;      // Vertex position (from Ocean::vertices VBO)
layout (location = 1) in vec3 aNormal;   // Vertex normal (from Ocean::normals VBO)
layout (location = 2) in vec2 aTexCoord; // Texture coordinates (from Ocean::texCoords VBO)

// Output to Fragment Shader
out vec2 TexCoord;
out vec3 FragPosWorld;     // Fragment position in world space
out vec3 NormalWorld;      // Normal vector in world space

out vec3 NormalInterp; // **OUT variable declaration - crucial!**

// Uniforms (matrices, light parameters, etc.)
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix; // Normal matrix for correct normal transformation

void main() {
    // 1. Transform vertex position to clip space
    gl_Position = projection * view  * vec4(aPos, 1.0);

    // 2. Pass texture coordinates to fragment shader
    TexCoord = aTexCoord;

    // 3. Calculate fragment position in world space
    FragPosWorld = vec3(model * vec4(aPos, 1.0));

    // 4. Transform normal vector to world space using the Normal Matrix
    NormalWorld = normalize(normalMatrix * aNormal);
}