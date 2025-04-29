#version 330 core
// Fragment Shader for Terrain Rendering

in vec3 NormalInterp; // **IN variable declaration - crucial!**


in vec2 TexCoord;
in vec3 FragPosWorld;
in vec3 NormalWorld;

out vec4 FragColor;

uniform sampler2D terrainTexture; // Sampler for terrain color texture (no normal map for now)
uniform sampler2D heightMapTexture; // **New: Sampler for heightmap texture**

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewPosWorld;

void main() {
    // 1. Sample terrain texture
    vec3 albedoColor = texture(terrainTexture, TexCoord).rgb;
    float heightValue = texture(heightMapTexture, TexCoord).r; // **Sample heightmap (Red channel = grayscale height)**

    // 2. Lighting calculations (Blinn-Phong - similar to ocean shader)
    vec3 normal = normalize(NormalWorld); // Use geometric normal for terrain
    vec3 lightDirNorm = normalize(lightDir);
    vec3 viewDirNorm = normalize(viewPosWorld - FragPosWorld);
    vec3 reflectDir = reflect(-lightDirNorm, normal);

    // Diffuse component
    float diff = max(dot(normal, lightDirNorm), 0.0);
    vec3 diffuse = diff * lightColor * albedoColor;

    // Specular component
    float spec = pow(max(dot(viewDirNorm, reflectDir), 0.0), 16.0); // Adjust shininess (exponent)
    vec3 specular = spec * lightColor * vec3(0.3); // Example specular color - less shiny than ocean

    // Ambient component
    vec3 ambient = 0.2 * lightColor * albedoColor;

    vec3 lowColor = vec3(0.2, 0.3, 0.05); // Darker green/brown for lower areas
    vec3 highColor = vec3(0.5, 0.7, 0.2); // Lighter green for higher areas
    vec3 heightBasedColor = mix(lowColor, highColor, heightValue); // Linear interpolation based on heightValue


    // 4. Combine lighting components for final color
    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(finalColor * heightBasedColor, 1.0);
}