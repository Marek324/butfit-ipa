/*
 * File:        render.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: Game renderer
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */



#include "Renderer.h"
#include <iostream>
#include <SOIL/SOIL.h>
#include "utils.h"
#include <glm/gtc/type_ptr.hpp>




Renderer::Renderer() : oceanTextureID(0), boatTextureID(0), terrainTextureID(0), heightMapTextureID(0) {}

Renderer::~Renderer() {}

bool Renderer::init()
{
    glewInit();
    if (glewIsSupported("GL_VERSION_3_0"))
    {
        std::cout << "OpenGL 3.0 or higher supported: Shaders will work." << std::endl;
    }
    else
    {
        std::cout << "OpenGL 3.0 or higher NOT supported: Shaders might not work." << std::endl;
    }

    glClearColor(0.529f, 0.808f, 0.922f, 1.0f); // Light blue background

    if (!loadTexture("assets/textures/ocean_texture.png", oceanTextureID))
    {
        std::cerr << "Error loading ocean texture." << std::endl;
        return false;
    }
    if (!loadTexture("assets/textures/rock_texture.png", terrainTextureID)) { // Assuming you name it terrain_texture.jpg
        std::cerr << "Error loading terrain texture." << std::endl;
        return false;
    }
    checkGLError("loadTexture - terrainTexture"); // Check after terrain texture loading

    // **Load heightmap texture:**
    if (!loadTexture("assets/textures/terain.png", heightMapTextureID)) { // Assuming heightmap is terrain_heightmap.png
        std::cerr << "Error loading terrain heightmap texture." << std::endl;
        return false;
    }
    checkGLError("loadTexture - heightMapTexture"); // Check after heightmap texture loading

    terrainShader.loadShader("assets/shaders/terrain_vertex_shader.glsl", "assets/shaders/terrain_fragment_shader.glsl");
    if (!terrainShader.isLoaded()) {
        std::cerr << "Error loading terrain shader program!" << std::endl;
        return false;
    }
    checkGLError("terrainShader.loadShader"); // Check after shader loading
    // Load and compile ocean shader program:
    oceanShader.loadShader("assets/shaders/ocean_vertex_shader.glsl", "assets/shaders/ocean_fragment_shader.glsl");
    if (!oceanShader.isLoaded()) {
        std::cerr << "Error loading ocean shader program!" << std::endl;
        return false;
    }
    checkGLError("oceanShader.loadShader"); // Check after shader loading
    // Boat texture is now loaded in drawBoat, based on Boat class texture path

    setupLighting();
    // shaderProgram.loadShader("assets/shaders/vertex_shader.glsl", "assets/shaders/fragment_shader.glsl"); // Optional shader loading

    return true;
}

void Renderer::cleanup()
{
    glDeleteTextures(1, &oceanTextureID);
    glDeleteTextures(1, &boatTextureID);
    glDeleteTextures(1, &terrainTextureID);
    glDeleteTextures(1, &heightMapTextureID);

    // shaderProgram.cleanup(); // Optional shader cleanup
}

void Renderer::renderScene(const Ocean &ocean, const Boat &boat, const Camera &camera, const Terrain &terrain)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    camera.lookAt(); // Set up camera view

    setupLighting(); // Ensure lighting is enabled each frame
    drawTerrain(terrain, camera); // **Call drawTerrain here - BEFORE drawOcean**

    drawOcean(ocean, camera);
    drawBoat(boat);
    checkGLError("drawTerrain"); // Check after drawTerrain
    // Optional: Render skybox, UI, etc.
    // ...
}

void Renderer::reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, static_cast<float>(width) / height, 0.1f, 100.0f); // Adjust near/far planes
    glMatrixMode(GL_MODELVIEW);
}

bool Renderer::loadTexture(const char *filename, GLuint &textureID)
{
    textureID = SOIL_load_OGL_texture(filename, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if (textureID == 0)
    {
        std::cerr << "SOIL loading error: '" << SOIL_last_result() << "' (" << filename << ")" << std::endl;
        return false;
    }

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D); // Generate mipmaps for better quality at distance
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture
    return true;
}

void Renderer::setupLighting()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat lightSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat lightPosition[] = {10.0f, 10.0f, 10.0f, 0.0f}; // Directional light from above and to the right

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); // Material colors track glColor
    GLfloat matSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat matShininess[] = {50.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShininess);
}

void Renderer::drawOcean(const Ocean& ocean, const Camera& camera)
{
    
    glPushMatrix();

    // Use the ocean shader program:
    oceanShader.use();
    checkGLError("oceanShader.use"); // Check after shader use

    // Set Uniforms for Ocean Shader:
    glm::mat4 modelMatrix;
    glm::mat4 projectionMatrix;

    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelMatrix));
    checkGLError("glGetFloatv(GL_MODELVIEW_MATRIX)"); // Check after glGetFloatv
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projectionMatrix));
    checkGLError("glGetFloatv(GL_PROJECTION_MATRIX)"); // Check after glGetFloatv


    oceanShader.setMat4("model", modelMatrix);
    oceanShader.setMat4("view", camera.getViewMatrix());
    oceanShader.setMat4("projection", projectionMatrix);
    oceanShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
    checkGLError("shader.setMat4/setMat3 uniforms"); // Check after setting matrix uniforms


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, oceanTextureID);
    oceanShader.setInt("oceanTexture", 0);
    checkGLError("glBindTexture - oceanTexture"); // Check after glBindTexture

    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, normalMapTextureID);
    //oceanShader.setInt("normalMap", 1);
    //checkGLError("glBindTexture - normalMapTexture"); // Check after glBindTexture


    oceanShader.setVec3("lightDir", glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)));
    oceanShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    oceanShader.setVec3("viewPosWorld", camera.getPosition());
    checkGLError("shader.setVec3 uniforms"); // Check after setting vec3 uniforms


    drawMeshVBO(ocean); // Draw using VBOs and IBO
    checkGLError("drawMeshVBO"); // Check after drawMeshVBO call

    // Unuse shader program after drawing ocean
    oceanShader.unuse();
    checkGLError("oceanShader.unuse"); // Check after shader unuse

    //glPopMatrix();

    glPushMatrix();
    //glTranslatef(0.0f, -1.0f, 0.0f); // Lower the ocean slightly

    // **Disable Texture and Lighting for Normal Visualization**
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for normals (you can change this)

    int gridSize = ocean.getGridSize();
    float gridSpacing = ocean.getGridSpacing();

    #if SHOW_NORM
    

    // **Draw Normals as Lines**
    glBegin(GL_LINES);
    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 v = ocean.getVertex(x, z);
            glm::vec3 normal = ocean.getWaveNormal(v.x, v.z, ocean.time); // Get normal at vertex

            // Calculate endpoint of normal line - Scale normal for visibility
            float normalLength = 0.5f; // Adjust this value to control normal line length
            glm::vec3 normalEndpoint = v + normal * normalLength;

            // Draw line representing the normal
            glVertex3f(v.x, v.y, v.z);                                        // Start point of normal line (vertex position)
            glVertex3f(normalEndpoint.x, normalEndpoint.y, normalEndpoint.z); // End point of normal line (along normal direction)
        }
    }
    glEnd();

    #endif

    // **Re-enable Texture and Lighting (if you want to switch back to textured rendering later)**
    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_LIGHTING);

    glPopMatrix();

    glPushMatrix();

    // **Disable Texture and Lighting for Wireframe Rendering**
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f); // Green color for wireframe (you can change this)




    #if SHOW_GRID

    // **Change glBegin mode to GL_LINES for Wireframe**
    glBegin(GL_LINES);
    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 v = ocean.getVertex(x, z);

            // Draw horizontal lines (along Z-axis)
            if (x < gridSize - 1)
            {
                glm::vec3 v_next_x = ocean.getVertex(x + 1, z);
                glVertex3f(v.x, v.y, v.z);
                glVertex3f(v_next_x.x, v_next_x.y, v_next_x.z);
            }
            // Draw vertical lines (along X-axis)
            if (z < gridSize - 1)
            {
                glm::vec3 v_next_z = ocean.getVertex(x, z + 1);
                glVertex3f(v.x, v.y, v.z);
                glVertex3f(v_next_z.x, v_next_z.y, v_next_z.z);
            }
        }
    }
    glEnd();

    #endif

    // **Re-enable Texture and Lighting (if you want to switch back to textured rendering later)**
    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();
}

// ... (rest of Renderer.cpp - Renderer::drawBoat, Renderer::drawMesh, etc.) ...

void Renderer::drawBoat(const Boat &boat)
{
    glPushMatrix();
    glm::vec3 boatPos = boat.getPosition();
    glm::quat boatRotation = boat.getRotation();
    float boatScale = boat.getScale(); // Get the boat's scale factor

    glTranslatef(boatPos.x, boatPos.y, boatPos.z);
    glm::mat4 rotationMatrix = glm::mat4_cast(boatRotation);
    glMultMatrixf(glm::value_ptr(rotationMatrix));

    glScalef(boatScale, boatScale, boatScale); // Uniform scaling - scale equally in all directions

    // Load boat texture here, based on the texture path from the Boat class
    if (!boat.getTexturePath().empty())
    {
        if (boatTextureID == 0 || boat.getTexturePath() != lastBoatTexturePath)
        { // Check if texture needs to be loaded or reloaded
            if (boatTextureID != 0)
            { // If there's a previous texture, delete it
                glDeleteTextures(1, &boatTextureID);
                boatTextureID = 0;
            }
            if (!loadTexture(boat.getTexturePath().c_str(), boatTextureID))
            {
                std::cerr << "Error loading boat texture: " << boat.getTexturePath() << std::endl;
            }
            else
            {
                lastBoatTexturePath = boat.getTexturePath(); // Store the path of the loaded texture
            }
        }
    }

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, boatTextureID);
    glColor3f(1.0f, 1.0f, 1.0f); // Texture color modulation

    // Draw the loaded boat mesh
    drawMesh(boat.getVertices(), boat.getNormals(), boat.getTexCoords(), boat.getMaterialIndices(), boat.getMaterials()); // Pass material data

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glm::vec3 minPoint = boat.getBoundingBoxMin(); // Get model-space min point
    glm::vec3 maxPoint = boat.getBoundingBoxMax(); // Get model-space max point

    glTranslatef(boatPos.x, boatPos.y, boatPos.z);
    glMultMatrixf(glm::value_ptr(rotationMatrix));
    glScalef(boatScale, boatScale, boatScale); // Uniform scaling - scale equally in all directions
                                               // **Draw Wireframe Bounding Box:**
    glDisable(GL_LIGHTING);                    // Disable lighting for bounding box (solid color wireframe)
    glColor3f(1.0f, 1.0f, 0.0f);               // Yellow color for bounding box
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Set polygon mode to line (wireframe)


    #if SHOW_BOUDING_BOX
    glBegin(GL_QUADS); // Draw a cube using quads (wireframe)

    // Front face
    glVertex3f(minPoint.x, minPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, minPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, maxPoint.y, maxPoint.z);
    glVertex3f(minPoint.x, maxPoint.y, maxPoint.z);

    // Back face
    glVertex3f(minPoint.x, minPoint.y, minPoint.z);
    glVertex3f(maxPoint.x, minPoint.y, minPoint.z);
    glVertex3f(maxPoint.x, maxPoint.y, minPoint.z);
    glVertex3f(minPoint.x, maxPoint.y, minPoint.z);

    // Top face
    glVertex3f(minPoint.x, maxPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, maxPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, maxPoint.y, minPoint.z);
    glVertex3f(minPoint.x, maxPoint.y, minPoint.z);

    // Bottom face
    glVertex3f(minPoint.x, minPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, minPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, minPoint.y, minPoint.z);
    glVertex3f(minPoint.x, minPoint.y, minPoint.z);

    // Right face
    glVertex3f(maxPoint.x, minPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, maxPoint.y, maxPoint.z);
    glVertex3f(maxPoint.x, maxPoint.y, minPoint.z);
    glVertex3f(maxPoint.x, minPoint.y, minPoint.z);

    // Left face
    glVertex3f(minPoint.x, minPoint.y, maxPoint.z);
    glVertex3f(minPoint.x, maxPoint.y, maxPoint.z);
    glVertex3f(minPoint.x, maxPoint.y, minPoint.z);
    glVertex3f(minPoint.x, minPoint.y, minPoint.z);

    glEnd();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Restore polygon mode to fill
    #endif

    glEnable(GL_LIGHTING);    
                     // Re-enable lighting for boat model
    glPopMatrix();
}


void Renderer::drawMeshVBO(const Ocean& ocean) {
    // 1. Bind Vertex Array Object (VAO) - if you are using VAOs. If not, bind VBOs directly.
    glBindVertexArray(ocean.getVAO()); // Assuming Ocean class has getVAO() that returns VAO ID

    // If NOT using VAOs, you would bind VBOs individually like this:
    /*
    glBindBuffer(GL_ARRAY_BUFFER, ocean.getVertexBufferID()); // Bind vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, ocean.getNormalBufferID());   // Bind normal VBO
    glBindBuffer(GL_ARRAY_BUFFER, ocean.getTexCoordBufferID()); // Bind texCoord VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ocean.getIndexBufferID()); // Bind IBO
    */


    // 2. Draw using glDrawElements (assuming you are using indexed rendering with IBO)
    glDrawElements(GL_QUADS, ocean.getIndexCount(), GL_UNSIGNED_INT, 0); // Draw using indices from IBO

    // If drawing as triangles instead of quads, use:
    // glDrawElements(GL_TRIANGLES, ocean.getIndexCount(), GL_UNSIGNED_INT, 0);


    // 3. Unbind VAO (or VBOs if not using VAOs) - optional, but good practice
    glBindVertexArray(0);

    // If NOT using VAOs, unbind VBOs individually (optional):
    /*
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    */
}

void Renderer::drawMeshTerainVBO(const Terrain& terrain) {
    // 1. Bind Vertex Array Object (VAO) - if you are using VAOs. If not, bind VBOs directly.
    glBindVertexArray(terrain.getVAO()); // Assuming Ocean class has getVAO() that returns VAO ID

    // If NOT using VAOs, you would bind VBOs individually like this:
    /*
    glBindBuffer(GL_ARRAY_BUFFER, ocean.getVertexBufferID()); // Bind vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, ocean.getNormalBufferID());   // Bind normal VBO
    glBindBuffer(GL_ARRAY_BUFFER, ocean.getTexCoordBufferID()); // Bind texCoord VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ocean.getIndexBufferID()); // Bind IBO
    */


    // 2. Draw using glDrawElements (assuming you are using indexed rendering with IBO)
    glDrawElements(GL_QUADS, terrain.getIndexCount(), GL_UNSIGNED_INT, 0); // Draw using indices from IBO

    // If drawing as triangles instead of quads, use:
    // glDrawElements(GL_TRIANGLES, ocean.getIndexCount(), GL_UNSIGNED_INT, 0);


    // 3. Unbind VAO (or VBOs if not using VAOs) - optional, but good practice
    glBindVertexArray(0);

    // If NOT using VAOs, unbind VBOs individually (optional):
    /*
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    */
}

// Static variable to store the last loaded boat texture path

void Renderer::drawMesh(const std::vector<glm::vec3> &vertices, const std::vector<glm::vec3> &normals, const std::vector<glm::vec2> &texCoords,
                        const std::vector<int> &materialIndices, const std::vector<tinyobj::material_t> &materials)
{
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < vertices.size(); ++i)
    {
        int material_index = materialIndices[i];
        if (material_index != -1 && static_cast<std::vector<tinyobj::material_t>::size_type>(material_index) < materials.size())
        { // Check if material index is valid
            const tinyobj::material_t &material = materials[material_index];
            glColor3f(material.diffuse[0], material.diffuse[1], material.diffuse[2]); // Set diffuse color
        }
        else
        {
            glColor3f(1.0f, 1.0f, 1.0f); // Default white color if no valid material
        }

        glNormal3f(normals[i].x, normals[i].y, normals[i].z);
        glTexCoord2f(texCoords[i].x, texCoords[i].y);
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
    }
    glEnd();
}

// Renderer.cpp - Add drawTerrain function
void Renderer::drawTerrain(const Terrain& terrain, const Camera& camera) {
    
    //printf("Drawing terrain\n");
    glPushMatrix();
    // No translation needed for terrain in this basic example
    glTranslatef(0.0f, 0.0f, 0.0f); // Lower the ocean slightly

    // Use the terrain shader program:
    terrainShader.use();
    checkGLError("terrainShader.use");

    glm::mat4 modelMatrix;
    glm::mat4 projectionMatrix;

    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelMatrix));
    checkGLError("glGetFloatv(GL_MODELVIEW_MATRIX)"); // Check after glGetFloatv
    glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projectionMatrix));
    checkGLError("glGetFloatv(GL_PROJECTION_MATRIX)"); // Check after glGetFloatv

    terrainShader.setMat4("model", modelMatrix);
    terrainShader.setMat4("view", camera.getViewMatrix());
    terrainShader.setMat4("projection", projectionMatrix);
    terrainShader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));
    checkGLError("terrainShader setMat4/setMat3 uniforms");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainTextureID);
    terrainShader.setInt("terrainTexture", 0);
    checkGLError("glBindTexture - terrainTexture");

    glActiveTexture(GL_TEXTURE1); // Activate texture unit 1 for heightmap
    glBindTexture(GL_TEXTURE_2D, heightMapTextureID); // Bind heightmap texture
    terrainShader.setInt("heightMapTexture", 1); // Set uniform sampler2D heightMapTexture to texture unit 1
    checkGLError("glBindTexture - heightMapTexture"); // Check after binding heightmap texture

    // Lighting Uniforms (reuse same light parameters as ocean for simplicity)
    terrainShader.setVec3("lightDir", glm::normalize(glm::vec3(1.0f, -1.0f, 1.0f))); // Example light direction
    terrainShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f)); // White light
    terrainShader.setVec3("viewPosWorld", camera.getPosition());
    checkGLError("terrainShader setVec3 uniforms");


    drawMeshTerainVBO(terrain); // Render terrain mesh using VBOs and IBO
    checkGLError("drawMeshVBO - terrain");
    glBindTexture(GL_TEXTURE_2D, 0); // Bind heightmap texture

    terrainShader.unuse();
    checkGLError("terrainShader.unuse");

    glPopMatrix();

    

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f); // Lower the ocean slightly

    // **Disable Texture and Lighting for Normal Visualization**
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for normals (you can change this)


    #if SHOW_NORM

    int gridSize = terrain.getGridSize();
    float gridSpacing = terrain.getGridSpacing();

    // **Draw Normals as Lines**
    glBegin(GL_LINES);
    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 v = terrain.getVertex(x, z);
            glm::vec3 normal = terrain.getNormal(x, z); // Get normal at vertex

            // Calculate endpoint of normal line - Scale normal for visibility
            float normalLength = 0.5f; // Adjust this value to control normal line length
            glm::vec3 normalEndpoint = v + normal * normalLength;

            // Draw line representing the normal
            glVertex3f(v.x, v.y, v.z);                                        // Start point of normal line (vertex position)
            glVertex3f(normalEndpoint.x, normalEndpoint.y, normalEndpoint.z); // End point of normal line (along normal direction)
        }
    }
    glEnd();

    #endif

    // **Re-enable Texture and Lighting (if you want to switch back to textured rendering later)**
    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_LIGHTING);

    glPopMatrix();



    glPushMatrix();
    glTranslatef(0.0f, 0.2f, 0.0f); // Lower the ocean slightly

    // **Disable Texture and Lighting for Wireframe Rendering**
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glColor3f(0.0f, 0.0f, 0.0f); // Green color for wireframe (you can change this)

    #if SHOW_GRID
    // **Change glBegin mode to GL_LINES for Wireframe**
    glBegin(GL_LINES);
    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            glm::vec3 v = terrain.getVertex(x, z);

            // Draw horizontal lines (along Z-axis)
            if (x < gridSize - 1)
            {
                glm::vec3 v_next_x = terrain.getVertex(x + 1, z);
                glVertex3f(v.x, v.y, v.z);
                glVertex3f(v_next_x.x, v_next_x.y, v_next_x.z);
            }
            // Draw vertical lines (along X-axis)
            if (z < gridSize - 1)
            {
                glm::vec3 v_next_z = terrain.getVertex(x, z + 1);
                glVertex3f(v.x, v.y, v.z);
                glVertex3f(v_next_z.x, v_next_z.y, v_next_z.z);
            }
        }
    }
    glEnd();

    #endif
    // **Re-enable Texture and Lighting (if you want to switch back to textured rendering later)**
    // glEnable(GL_TEXTURE_2D);
    // glEnable(GL_LIGHTING);

    glPopMatrix();
}