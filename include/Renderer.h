// Renderer.h
#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Ocean.h"
#include "Boat.h"
#include "Camera.h"
#include "Shader.h" // Optional Shader class
#include "Terrain.h" // Include Terrain header


#define SHOW_GRID 0
#define SHOW_NORM 0
#define SHOW_BOUDING_BOX 1

class Renderer {
public:
    Renderer();
    ~Renderer();
    GLuint oceanTextureID;
    GLuint boatTextureID;
    GLuint terrainTextureID; // Texture ID for terrain
    GLuint heightMapTextureID; // **Add heightMapTextureID**
    bool init();
    void cleanup();
    void renderScene(const Ocean &ocean, const Boat &boat, const Camera &camera, const Terrain &Terrain);
    void reshape(int width, int height);
    void drawOcean(const Ocean& ocean, const Camera& camera); // Camera argument added
    GLuint getTerrainTextureID() const { return terrainTextureID; } // Getter for terrain texture ID

private:

    // Shader shaderProgram; // Optional shader program
    GLuint normalMapTextureID; 
    Shader oceanShader; // Shader program for ocean
    Shader terrainShader; // **Add terrainShader member variable**

    std::string lastBoatTexturePath = "";

    bool loadTexture(const char* filename, GLuint& textureID);
    void setupLighting();
    void drawBoat(const Boat& boat);
    void drawMesh(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& texCoords,
                  const std::vector<int>& materialIndices, const std::vector<tinyobj::material_t>& materials); // Modified drawMesh

    void drawMeshVBO(const Ocean& ocean); // **Declare drawMeshVBO**
    void drawTerrain(const Terrain& terrain, const Camera& camera);
    void drawMeshTerainVBO(const Terrain& terrain);
};

#endif // RENDERER_H