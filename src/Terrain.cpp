/*
 * File:        Terran.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: This class defines terrain (surface).
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */


#include "Terrain.h"
#include <cmath>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include "utils.h" // Include utils.h for checkGLError
#include <GL/glew.h>

Terrain::Terrain(int gridSize, float gridSpacing)
    : gridSize(gridSize),
      gridSpacing(gridSpacing),
      vertices(gridSize * gridSize),
      normals(gridSize * gridSize),
      texCoords(gridSize * gridSize),
      vertexBufferID(0),
      normalBufferID(0),
      texCoordBufferID(0),
      indexBufferID(0),
      vaoID(0),
      indexCount(0) {}

Terrain::~Terrain() {
    cleanup();
}

bool Terrain::init(GLuint heightMapTextureID) {
    generateGrid(heightMapTextureID);
    createBuffers();
    return true;
}

void Terrain::cleanup() {
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &normalBufferID);
    glDeleteBuffers(1, &texCoordBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vaoID);
}



void Terrain::generateGrid(GLuint heightMapTextureID) {

    std::cout << "Terrain::generateGrid - gridSize: " << gridSize << std::endl;
    vertices.resize(gridSize * gridSize);
    normals.resize(gridSize * gridSize);
    texCoords.resize(gridSize * gridSize);

    // **Access Heightmap Texture Data**
    glBindTexture(GL_TEXTURE_2D, heightMapTextureID); // Bind heightmap texture
    GLint textureWidth, textureHeight;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth); // Get texture width
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight); // Get texture height

    std::vector<unsigned char> heightmapData(textureWidth * textureHeight); // Assuming 8-bit grayscale heightmap
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, heightmapData.data()); // Get texture pixel data (Red channel = grayscale height)
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture

    float heightScale = 40.0f; // Adjust this to control terrain height scale

    for (int x = 0; x < gridSize; ++x) {
        for (int z = 0; z < gridSize; ++z) {
            float worldX = (x - gridSize / 2.0f) * gridSpacing;
            float worldZ = (z - gridSize / 2.0f) * gridSpacing;

            // **Sample height from heightmap texture**
            float height = 0.0f;
            if (x < textureWidth && z < textureHeight && x >= 0 && z >= 0) { // Bounds check - important!
                height = static_cast<float>(heightmapData[z * textureWidth + x]) / 255.0f * heightScale-15.0; // Normalize to [0, 1], scale by heightScale

            } else {
                // Handle out-of-bounds access (e.g., set height to 0 or a default value)
                height = 0.0f;
            }

            //vertices[x * gridSize + z] = glm::vec3(worldX, height, worldZ); // Set vertex position with height from heightmap
            vertices[x * gridSize + z] = glm::vec3(worldX, height, worldZ); // Flat terrain at y=0
            normals[x * gridSize + z] = glm::vec3(0.0f, 1.0f, 0.0f);    // Upward normals for flat terrain
            texCoords[x * gridSize + z] = glm::vec2(static_cast<float>(x) / 50.0f, static_cast<float>(z) / 50.0f); // Example texture tiling
            //originalWorldX[x * gridSize + z] = worldX;
            //originalWorldZ[x * gridSize + z] = worldZ;
        }
    }
    std::cout << "Width: " << textureWidth << "Geight: " << textureHeight << std::endl;
}


void Terrain::createBuffers() {
    glGenVertexArrays(1, &vaoID);
    checkGLError("1"); // Check after drawMeshVBO call

    glBindVertexArray(vaoID);
    checkGLError("2"); // Check after drawMeshVBO call

    glGenBuffers(1, &vertexBufferID);
    checkGLError("3"); // Check after drawMeshVBO call

    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    checkGLError("4"); // Check after drawMeshVBO call

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
    checkGLError("5"); // Check after drawMeshVBO call

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    checkGLError("6"); // Check after drawMeshVBO call

    glEnableVertexAttribArray(0);
    checkGLError("7"); // Check after drawMeshVBO call

    glGenBuffers(1, &normalBufferID);
    checkGLError("8"); // Check after drawMeshVBO call

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    checkGLError("9"); // Check after drawMeshVBO call

    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    checkGLError("10"); // Check after drawMeshVBO call

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    checkGLError("11"); // Check after drawMeshVBO call

    glEnableVertexAttribArray(1);
    checkGLError("12"); // Check after drawMeshVBO call


    glGenBuffers(1, &texCoordBufferID);
    checkGLError("13"); // Check after drawMeshVBO call

    glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferID);
    checkGLError("14"); // Check after drawMeshVBO call

    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    checkGLError("15"); // Check after drawMeshVBO call

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
    checkGLError("16"); // Check after drawMeshVBO call

    glEnableVertexAttribArray(2);
    checkGLError("17"); // Check after drawMeshVBO call


    std::vector<unsigned int> indices;
    for (int x = 0; x < gridSize - 1; ++x) {
        for (int z = 0; z < gridSize - 1; ++z) {
            unsigned int v00 = x * gridSize + z;
            unsigned int v10 = (x + 1) * gridSize + z;
            unsigned int v11 = (x + 1) * gridSize + (z + 1);
            unsigned int v01 = x * gridSize + (z + 1);
            indices.insert(indices.end(), {v00, v10, v11, v01});
        }
    }
    glGenBuffers(1, &indexBufferID);
    checkGLError("glGenBuffers - indexBufferID"); // Check after glGenBuffers
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    checkGLError("18"); // Check after drawMeshVBO call

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    checkGLError("19"); // Check after drawMeshVBO call

    glBindVertexArray(0);
    checkGLError("20"); // Check after drawMeshVBO call

    indexCount = indices.size();
}

void Terrain::updateBuffers() {
    // Not needed for static terrain in this basic example
}


glm::vec3 Terrain::getVertex(int x, int z) const {
    assert(x >= 0 && x < gridSize);
    assert(z >= 0 && z < gridSize);
    return vertices[x * gridSize + z];
}


glm::vec3 Terrain::getNormal(float x, float z) const
{
    return normals[x * gridSize + z];

}
