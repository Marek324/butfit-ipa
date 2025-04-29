/*
 * File:        Ocean.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: This class defines ocean (surface). The core functions of this class are optimized by the IPA Project 2025.
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */

#include "Ocean.h"
#include <cmath>
#include <glm/gtc/constants.hpp> // For pi
#include <chrono>

#if ASM_TYPE == CLEAR_ASM
// Assembly version declaration (signature changed to float arrays)
extern "C" void updateVertices_simd(float *updatedVertices_array, float *updatedNormals_array, size_t numVertices, float *originalWorldX_, float *originalWorldZ_, int _grid_size, float time, float *waves, int waveCount);

#else

// C++ implementation of SIMD version (now also taking float arrays for consistency)
void Ocean::updateVertices_simd(float *updatedVertices_array, float *updatedNormals_array, size_t numVertices, float *originalWorldX_, float *originalWorldZ_, int _grid_size, float time)
{
    // Placeholder C++ implementation using float arrays
    // for (size_t i = 0; i < numVertices; ++i) {
    //    updatedVertices_array[i * 3 + 0] += time * 0.1f; // Example: simple movement in x-direction (modifying float array directly)
    //}
    // Update normals in the float array as needed based on your simulation
}
#endif

Ocean::Ocean(int gridSize) : time(0.0f), gridSize(gridSize), gridSpacing(1.0f),
                             amplitude(0.8f), wavelength(10.0f), frequency(1.0f), // Adjusted amplitude slightly
                             direction(glm::vec2(1.0f, 0.0f)), phase(0.0f)
{
    gerstnerWaves.push_back({1.0f, 10.0f, 1.0f, glm::normalize(glm::vec2(1.0f, 0.0f)), 0.0f});
    gerstnerWaves.push_back({0.3f, 5.0f, 2.0f, glm::normalize(glm::vec2(1.0f, 1.0f)), 0.0f});
    // gerstnerWaves.push_back({1.0f, 3.0f, 1.0f, glm::normalize(glm::vec2(1.0f, 0.5f)), 0.0f});
    // gerstnerWaves.push_back({0.3f, 5.0f, 2.0f, glm::normalize(glm::vec2(0.5f, 0.5f)), 0.0f});
    // gerstnerWaves.push_back({1.0f, 10.0f, 1.0f, glm::normalize(glm::vec2(1.0f, 0.0f)), 0.0f});
    // gerstnerWaves.push_back({0.5f, 2.0f, 3.0f, glm::normalize(glm::vec2(1.0f, 1.0f)), 0.0f});
    // gerstnerWaves.push_back({1.0f, 1.0f, 0.2f, glm::normalize(glm::vec2(0.7f, 0.2f)), 0.0f});
    // gerstnerWaves.push_back({0.5f, 1.2f, 2.0f, glm::normalize(glm::vec2(0.9f, 0.8f)), 0.0f});
}

bool Ocean::init()
{
    generateGrid();
    createBuffers(); // Create VBOs and IBO

    return true;
}

void convert_gerstner_to_float_array(const std::vector<GerstnerWave> &waves, float *dst)
{
    size_t numVertices = waves.size();

    for (size_t i = 0; i < numVertices; ++i)
    {
        dst[i * 6 + 0] = waves[i].amplitude;
        dst[i * 6 + 1] = waves[i].wavelength;
        dst[i * 6 + 2] = waves[i].speed;
        dst[i * 6 + 2] = waves[i].direction.x;
        dst[i * 6 + 2] = waves[i].direction.y;
        dst[i * 6 + 2] = waves[i].phase;
    }
}

float min_vert_err = FLT_MAX;
float max_vert_err = 0;
float min_norm_err = FLT_MAX;
float max_norm_err = 0;

void Ocean::update(float deltaTime)
{
    time += deltaTime;
    std::vector<glm::vec3> updatedVertices_vec = vertices;      // Create a copy to update (vector version)
    std::vector<glm::vec3> updatedNormals_vec(vertices.size()); // Vector to store updated normals (vector version)

    std::vector<glm::vec3> updatedVertices_simd_vec = vertices;      // Create a copy to update (vector for comparison)
    std::vector<glm::vec3> updatedNormals_simd_vec(vertices.size()); // Vector to store updated normals (vector for comparison)

    // --- Conversion to Float Arrays for SIMD function ---
    size_t numVertices = vertices.size();
    size_t floatArraySize = numVertices * 3;

    // Reference C++ part
    auto start_time = std::chrono::high_resolution_clock::now();
    uint64_t start = rdtsc();
    // --- Call C++ version for comparison (using vector) ---
    // updateVertices(&updatedVertices_vec, &updatedNormals_vec, time);
    updateVertices(&updatedVertices_vec, &updatedNormals_vec, originalWorldX.data(), originalWorldZ.data(), gridSize, time);
    updateBuffers(updatedVertices_vec, updatedNormals_vec); // Use vectors for updateBuffers
    uint64_t end = rdtsc();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    std::cout << "Ocean::update took " << duration.count() << "ns " << "CPU cycles: " << (end - start) << std::endl;
    // End Reference C++ part

    // own C++ re-implementationto better understand the algorithm and locate redundant calculations
    std::vector<glm::vec3> o_updatedVertices_vec = vertices;      // Create a copy to update (vector version)
    std::vector<glm::vec3> o_updatedNormals_vec(vertices.size()); // Vector to store updated normals (vector version)

    start_time = std::chrono::high_resolution_clock::now();
    start = rdtsc();

    own_cpp_updateVertices(&o_updatedVertices_vec, &o_updatedNormals_vec, originalWorldX.data(), originalWorldZ.data(), gridSize, time);

    end = rdtsc();
    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    std::cout << "Ocean::update (own) took " << duration.count() << "ns " << "CPU cycles: " << (end - start) << std::endl;
    std::cout << "Own implementation check " << (((o_updatedVertices_vec == updatedVertices_vec) && (o_updatedNormals_vec == updatedNormals_vec)) ? "OK" : "NOT OK") << std::endl;
    if (o_updatedVertices_vec != updatedVertices_vec)
    {
        double totalErr = 0;
        for (int i = 0; i< o_updatedVertices_vec.size(); i++)
        {
            totalErr += abs(o_updatedVertices_vec.at(i).y - updatedVertices_vec.at(i).y);
        }
        min_vert_err = min_vert_err > totalErr ? totalErr : min_vert_err;
        max_vert_err = max_vert_err < totalErr ? totalErr : max_vert_err;
        std::cout << "vert totalErr: " << totalErr << " vec size: " << o_updatedVertices_vec.size() << "\n"
        << "min: " << min_vert_err << " max: " << max_vert_err << std::endl;
    }

    if (o_updatedNormals_vec != updatedNormals_vec)
    {
        double totalErr = 0;
        for (int i = 0; i< o_updatedNormals_vec.size(); i++)
        {
            totalErr += abs(o_updatedNormals_vec.at(i).y - updatedNormals_vec.at(i).y);
        }
        min_norm_err = min_norm_err > totalErr ? totalErr : min_norm_err;
        max_norm_err = max_norm_err < totalErr ? totalErr : max_norm_err;
        std::cout << "norm totalErr: " << totalErr << " vec size: " << o_updatedNormals_vec.size() << "\n"
        << "min: " << min_norm_err << " max: " << max_norm_err << std::endl;
    }

    float *updatedVertices_simd_array = new float[floatArraySize]; // Array for SIMD function
    float *updatedNormals_simd_array = new float[floatArraySize];  // Array for SIMD function
    // end own

    // Start SIMD part
    //  Convert vector of vec3 to float array (for both normal and simd versions)

    convert_vec3_to_float_array(updatedVertices_simd_vec, updatedVertices_simd_array); // Convert base vertices
    convert_vec3_to_float_array(updatedNormals_simd_vec, updatedNormals_simd_array);   // Convert base vertices

    start_time = std::chrono::high_resolution_clock::now();
    start = rdtsc();

    // READ
    // Extend function parameters by passing a reference to a structure with
    // Gerstner wave properties, allowing customization through your own implementation.

    // inspired by vertices/normals conversion
    size_t num_waves = gerstnerWaves.size();
    float *converted_waves = new float[num_waves * 6];
    convert_gerstner_to_float_array(gerstnerWaves, converted_waves);

    updateVertices_simd(updatedVertices_simd_array, updatedNormals_simd_array, numVertices, originalWorldX.data(), originalWorldZ.data(), gridSize, time, converted_waves, num_waves);

    end = rdtsc();

    end_time = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);

    std::cout << "Ocean::update (SIMD) took " << duration.count() << "ns " << "CPU cycles: " << (end - start) << std::endl;

    // --- Convert float arrays back to vectors for updateBuffers (if needed) ---
    std::vector<glm::vec3> updatedVertices_vec_from_array(numVertices);
    std::vector<glm::vec3> updatedNormals_vec_from_array(numVertices);

    convert_float_array_to_vec3(updatedVertices_simd_array, updatedVertices_vec_from_array);
    convert_float_array_to_vec3(updatedNormals_simd_array, updatedNormals_vec_from_array);

    // READ

    // THIS FUNCTION UPDATES THE VERTICES AND NORMALS. If updateVertices_simd doesn't work,
    // the surface will remain flat. Uncomment this after implementing the update logic.

    // updateBuffers(updatedVertices_vec_from_array, updatedNormals_vec_from_array); // Use vectors for updateBuffers

    // End SIMD part

    // --- Deallocate float arrays ---
    delete[] converted_waves;
    delete[] updatedVertices_simd_array;
    delete[] updatedNormals_simd_array;
}

void Ocean::generateGrid()
{
    // std::cout << "Ocean::generateGrid - gridSize: " << gridSize << " " << vertices.size()<< std::endl;
    vertices.resize(gridSize * gridSize);
    // Resize original coordinate vectors
    originalWorldX.resize(gridSize * gridSize);
    originalWorldZ.resize(gridSize * gridSize);

    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            float worldX = (x - gridSize / 2.0f) * gridSpacing;
            float worldZ = (z - gridSize / 2.0f) * gridSpacing;
            vertices[x * gridSize + z] = glm::vec3(worldX, 0.0f, worldZ);

            // Store original world coordinates
            originalWorldX[x * gridSize + z] = worldX;
            originalWorldZ[x * gridSize + z] = worldZ;
        }
    }
}

void Ocean::updateVertices(std::vector<glm::vec3> *updatedVertices, std::vector<glm::vec3> *updatedNormals, float *originalWorldX_, float *originalWorldZ_, int _grid_size, float time)
{

    for (int x = 0; x < _grid_size; ++x)
    {
        for (int z = 0; z < _grid_size; ++z)
        {

            glm::vec3 &vertex = (*updatedVertices)[x * _grid_size + z]; // Use reference to modify directly
            float originalX = originalWorldX_[x * _grid_size + z];
            float originalZ = originalWorldZ_[x * _grid_size + z];
            vertex.y = getWaveHeight(vertex.x, vertex.z, time);

            (*updatedNormals)[x * gridSize + z] = getWaveNormal(originalX, originalZ, time); // Calculate normal using original coords
        }
    }
}

float Ocean::getWaveHeight(float x, float z, float time) const
{
    float totalHeight = 0.0f;
    for (const auto &wave : gerstnerWaves)
    {
        totalHeight += getGerstnerWaveHeight(wave, x, z, time);
    }
    return totalHeight;
}

float Ocean::getGerstnerWaveHeight(const GerstnerWave &wave, float x, float z, float time) const
{
    float k = 2.0f * glm::pi<float>() / wave.wavelength;
    float w = wave.speed * k;
    float dotProduct = glm::dot(wave.direction, glm::vec2(x, z));
    float periodicAmplitude = wave.amplitude * 0.5f * (1.0f + sin(2.0f * glm::pi<float>() * time / wave.wavelength));
    float waveHeightValue = periodicAmplitude * sin(k * dotProduct - w * time + wave.phase);

    if (fabs(x) < 0.5f && fabs(z) < 0.5f)
    {
        // std::cout << "  getGerstnerWaveHeight - time: " << time << ", periodicAmplitude: " << periodicAmplitude << ", waveHeightValue: " << waveHeightValue << std::endl;
    }

    return waveHeightValue;
}

glm::vec3 Ocean::getGerstnerWaveDisplacement(const GerstnerWave &wave, float x, float z, float time) const
{
    float k = 2.0f * glm::pi<float>() / wave.wavelength;
    float w = wave.speed * k;
    float dotProduct = glm::dot(wave.direction, glm::vec2(x, z));
    float cosTerm = cos(k * dotProduct - w * time + wave.phase);
    float periodicAmplitude = wave.amplitude * 0.5f * (1.0f + sin(2.0f * glm::pi<float>() * time / wave.wavelength));
    return glm::vec3(wave.direction.x * periodicAmplitude * cosTerm,
                     0.0f,
                     wave.direction.y * periodicAmplitude * cosTerm);
}

glm::vec3 Ocean::getWaveNormal(float x, float z, float time) const
{
    glm::vec3 tangentX = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 tangentZ = glm::vec3(0.0f, 0.0f, 1.0f);

    for (const auto &wave : gerstnerWaves)
    {
        float k = 2.0f * glm::pi<float>() / wave.wavelength;
        float w = wave.speed * k;
        float dotProduct = glm::dot(wave.direction, glm::vec2(x, z));
        float sinTerm = sin(k * dotProduct - w * time + wave.phase);
        float cosTerm = cos(k * dotProduct - w * time + wave.phase);
        float periodicAmplitude = wave.amplitude * 0.5f * (1.0f + sin(2.0f * glm::pi<float>() * time / wave.wavelength));
        float modulatedAmplitude = periodicAmplitude;

        // Calculate tangent vectors for EACH wave component and ACCUMULATE them directly
        tangentX += glm::vec3(
            -modulatedAmplitude * wave.direction.x * wave.direction.x * k * sinTerm, // dx_dx
            modulatedAmplitude * wave.direction.x * k * cosTerm,                     // dy_dx
            -modulatedAmplitude * wave.direction.x * wave.direction.y * k * sinTerm  // dz_dx
        );

        tangentZ += glm::vec3(
            -modulatedAmplitude * wave.direction.x * wave.direction.y * k * sinTerm, // dx_dz
            modulatedAmplitude * wave.direction.y * k * cosTerm,                     // dy_dz
            -modulatedAmplitude * wave.direction.y * wave.direction.y * k * sinTerm  // dz_dz
        );
    }

    return glm::normalize(glm::cross(tangentZ, tangentX));
}

void Ocean::updateBuffers(const std::vector<glm::vec3> &updatedVertices, const std::vector<glm::vec3> &updatedNormals)
{
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, updatedVertices.size() * sizeof(glm::vec3), updatedVertices.data()); // Update vertex positions

    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, updatedNormals.size() * sizeof(glm::vec3), updatedNormals.data()); // Update normals
}

glm::vec3 Ocean::getVertex(int x, int z) const
{
    return vertices[x * gridSize + z];
}

void Ocean::setGridSize(int newGridSize)
{
    gridSize = newGridSize;
    generateGrid(); // Re-generate the grid with the new size

    std::vector<glm::vec3> updatedVertices = vertices;      // Create a copy to update
    std::vector<glm::vec3> updatedNormals(vertices.size()); // Vector to store updated normals

    updateVertices(&updatedVertices, &updatedNormals, originalWorldX.data(), originalWorldZ.data(), gridSize, time);

    updateBuffers(updatedVertices, updatedNormals);
    // updateVertices(); // Re-update vertices based on waves (optional, if needed immediately)
}

GLuint Ocean::getVAO() const
{
    return vaoID;
}

GLuint Ocean::getIndexCount() const
{
    return indexCount;
}

int Ocean::getGridIndex(int x, int z) const
{
    return x * gridSize + z;
}

void Ocean::createBuffers()
{
    glGenVertexArrays(1, &vaoID);
    checkGLError("glGenVertexArrays"); // Check after glGenVertexArrays
    glBindVertexArray(vaoID);
    checkGLError("glBindVertexArray"); // Check after glBindVertexArray

    // Generate VBOs
    glGenBuffers(1, &vertexBufferID);
    checkGLError("glGenBuffers - vertexBufferID"); // Check after glGenBuffers
    glGenBuffers(1, &normalBufferID);
    checkGLError("glGenBuffers - normalBufferID"); // Check after glGenBuffers
    glGenBuffers(1, &texCoordBufferID);
    checkGLError("glGenBuffers - texCoordBufferID"); // Check after glGenBuffers
    glGenBuffers(1, &indexBufferID);
    checkGLError("glGenBuffers - indexBufferID"); // Check after glGenBuffers

    // 1. Vertex Positions VBO
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    checkGLError("glBindBuffer - vertexBufferID"); // Check after glBindBuffer
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    checkGLError("glBufferData - vertexBufferID"); // Check after glBufferData
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    checkGLError("glVertexAttribPointer - vertexBufferID"); // Check after glVertexAttribPointer
    glEnableVertexAttribArray(0);
    checkGLError("glEnableVertexAttribArray - location 0"); // Check after glEnableVertexAttribArray

    // 2. Vertex Normals VBO (initially flat normals - updated in updateVertices/updateBuffers)
    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f, 1.0f, 0.0f)); // Initialize with flat normals
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    checkGLError("glBindBuffer - normalBufferID"); // Check after glBindBuffer
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    checkGLError("glBufferData - normalBufferID"); // Check after glBufferData
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    checkGLError("glVertexAttribPointer - normalBufferID"); // Check after glVertexAttribPointer
    glEnableVertexAttribArray(1);
    checkGLError("glEnableVertexAttribArray - location 1"); // Check after glEnableVertexAttribArray

    // 3. Texture Coordinates VBO
    std::vector<glm::vec2> texCoords(vertices.size());
    for (int x = 0; x < gridSize; ++x)
    {
        for (int z = 0; z < gridSize; ++z)
        {
            float texU = static_cast<float>(x) / 70.0f;
            float texV = static_cast<float>(z) / 70.0f;
            texCoords[x * gridSize + z] = glm::vec2(texU, texV);
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, texCoordBufferID);
    checkGLError("glBindBuffer - texCoordBufferID"); // Check after glBindBuffer
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    checkGLError("glBufferData - texCoordBufferID"); // Check after glBufferData
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    checkGLError("glVertexAttribPointer - texCoordBufferID"); // Check after glVertexAttribPointer
    glEnableVertexAttribArray(2);
    checkGLError("glEnableVertexAttribArray - location 2"); // Check after glEnableVertexAttribArray

    // 4. Index Buffer Object (IBO) for Quads
    std::vector<unsigned int> indices;
    for (int x = 0; x < gridSize - 1; ++x)
    {
        for (int z = 0; z < gridSize - 1; ++z)
        {
            unsigned int v00 = x * gridSize + z;
            unsigned int v10 = (x + 1) * gridSize + z;
            unsigned int v11 = (x + 1) * gridSize + (z + 1);
            unsigned int v01 = x * gridSize + (z + 1);
            indices.insert(indices.end(), {v00, v10, v11, v01}); // Quad indices (counter-clockwise)
        }
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    checkGLError("glBindBuffer - indexBufferID"); // Check after glBindBuffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    checkGLError("glBufferData - indexBufferID"); // Check after glBufferData

    glBindVertexArray(0);                                   // Unbind VAO
    checkGLError("glBindVertexArray(0)");                   // Check after unbinding VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);                       // Unbind VBO - Optional, VAO unbinding often unbinds VBOs
    checkGLError("glBindBuffer(0) - ARRAY_BUFFER");         // Check after unbinding ARRAY_BUFFER
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);               // Unbind IBO - Optional, VAO unbinding often unbinds IBO
    checkGLError("glBindBuffer(0) - ELEMENT_ARRAY_BUFFER"); // Check after unbinding ELEMENT_ARRAY_BUFFER
    indexCount = indices.size();                            // Store index count for rendering
}

Ocean::~Ocean()
{
    cleanup(); // Call cleanup to release OpenGL resources
}

void Ocean::cleanup()
{
    // No dynamic memory allocation in this simple version
    // Release OpenGL resources (VBOs, IBO, VAO)
    glDeleteBuffers(1, &vertexBufferID);
    glDeleteBuffers(1, &normalBufferID);
    glDeleteBuffers(1, &texCoordBufferID);
    glDeleteBuffers(1, &indexBufferID);
    glDeleteVertexArrays(1, &vaoID);
    vertexBufferID = 0;
    normalBufferID = 0;
    texCoordBufferID = 0;
    indexBufferID = 0;
    vaoID = 0;
}

void Ocean::own_cpp_updateVertices(std::vector<glm::vec3> *updatedVertices, std::vector<glm::vec3> *updatedNormals, float *originalWorldX_, float *originalWorldZ_, int _grid_size, float time)
{
    for (int x = 0; x < _grid_size; ++x)
    {
        for (int z = 0; z < _grid_size; ++z)
        {

            glm::vec3 &vertex = (*updatedVertices)[x * _grid_size + z]; // Use reference to modify directly
            float originalX = originalWorldX_[x * _grid_size + z];
            float originalZ = originalWorldZ_[x * _grid_size + z];

            float total_height = 0.0f;

            glm::vec3 tangentX = glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 tangentZ = glm::vec3(0.0f, 0.0f, 1.0f);

            for (const auto &wave : gerstnerWaves)
            {
                // used in both original functions
                float k = 2.0f * glm::pi<float>() / wave.wavelength;
                float periodicAmplitude = wave.amplitude * 0.5f * (1.0f + sin(k * time));
                float amp_k = periodicAmplitude * k;
                float dotProduct = glm::dot(wave.direction, glm::vec2(originalX, originalZ));
                // float dotProductHeight = glm::dot(wave.direction, glm::vec2(vertex.x, vertex.z)); // should be using this for height ??
                float w = wave.speed * k;
                float phase = k * dotProduct - w * time + wave.phase;
                // w, dotProduct, k -
                float sinTerm = sin(phase);
                float waveHeightValue = periodicAmplitude * sinTerm;
                // periodicAmplitude, -
                total_height += waveHeightValue;
                // waveHeightValue -
                float cosTerm = cos(phase);
                // phase -

                float amp_dir_x = amp_k * wave.direction.x;
                float amp_x_sin = -amp_dir_x * sinTerm;
                amp_dir_x *= cosTerm;

                tangentX += glm::vec3(
                    amp_x_sin * wave.direction.x,
                    amp_dir_x,
                    amp_x_sin * wave.direction.y);
                // amp_dir_x, amp_x_sin -

                float amp_dir_y = amp_k * wave.direction.y;
                // amp_k -
                float amp_y_sin = -amp_dir_y * sinTerm;
                // sinTerm -
                amp_dir_y *= cosTerm;
                // cosTerm -

                tangentZ += glm::vec3(
                    amp_y_sin * wave.direction.x,
                    amp_dir_y,
                    amp_y_sin * wave.direction.y);
                // amp_dir_y, amp_y_sin -
            }

            vertex.y = total_height;
            // total_height -
            (*updatedNormals)[x * _grid_size + z] = glm::normalize(glm::cross(tangentZ, tangentX));
        }
    }
}
