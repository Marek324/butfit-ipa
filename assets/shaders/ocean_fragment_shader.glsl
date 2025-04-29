#version 330 core
// Fragment Shader for Ocean Rendering with Texture Normals

in vec3 NormalInterp; // **IN variable declaration - crucial!**

// Input from Vertex Shader
in vec2 TexCoord;
in vec3 FragPosWorld;
in vec3 NormalWorld;

// Output fragment color
out vec4 FragColor;

// Uniforms (textures, lighting parameters, camera position)
uniform sampler2D oceanTexture;    // Sampler for ocean color texture
uniform sampler2D normalMap;       // Sampler for normal map texture
uniform vec3 lightDir;          // Directional light direction (world space)
uniform vec3 lightColor;        // Light color
uniform vec3 viewPosWorld;      // Camera position in world space

void main() {
    // 1. Sample textures
    vec3 albedoColor = texture(oceanTexture, TexCoord).rgb; // Sample ocean color texture
    //vec3 normalMapSample = texture(normalMap, TexCoord).rgb; // Sample normal map

    // 2. Unpack and transform normal from normal map (Tangent Space to World Space - Simplified)
    //vec3 normalMapNormal = normalize(normalMapSample * 2.0 - 1.0); // Unpack from [0, 1] to [-1, 1] and normalize
    vec3 normalWorld = normalize(NormalInterp); // Get interpolated geometric normal from vertex shader

    // **Basic Tangent Space Normal Mapping Approximation:**
    // For truly correct tangent-space normal mapping, you'd need to construct a proper TBN matrix.
    vec3 finalNormal = normalize(normalWorld ); // **Blend/Perturb geometric normal with texture normal**


    // 3. Lighting calculations (Blinn-Phong example)
    vec3 lightDirNorm = normalize(lightDir);
    vec3 viewDirNorm = normalize(viewPosWorld - FragPosWorld);
    vec3 reflectDir = reflect(-lightDirNorm, finalNormal);

    // Diffuse component
    float diff = max(dot(finalNormal, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * albedoColor;

    // Specular component (Blinn-Phong)
    float spec = pow(max(dot(viewDirNorm, reflectDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * vec3(0.8); // Example specular color

    // Ambient component
    vec3 ambient = 0.3 * lightColor * albedoColor; // Example ambient

    // 4. Combine lighting components for final color
    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(finalColor, 1.0); // Output final fragment color
}