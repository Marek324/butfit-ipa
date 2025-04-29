/*
 * File:        utils.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: Utils - perlinNoise is does not use in this project
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */


#include "utils.h"


uint64_t rdtsc() {
    return __rdtsc();  // Read Time-Stamp Counter
}

void checkGLError(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error after " << operation << ": ";
        switch (error) {
            case GL_INVALID_ENUM:
                std::cerr << "GL_INVALID_ENUM - An unacceptable value is specified for an enumerated argument.";
                break;
            case GL_INVALID_VALUE:
                std::cerr << "GL_INVALID_VALUE - A numeric argument is out of range.";
                break;
            case GL_INVALID_OPERATION:
                std::cerr << "GL_INVALID_OPERATION - The specified operation is not allowed in the current state.";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION - The framebuffer object is not complete.";
                break;
            case GL_OUT_OF_MEMORY:
                std::cerr << "GL_OUT_OF_MEMORY - There is not enough memory left to execute the command.";
                break;
            case GL_STACK_UNDERFLOW:
                std::cerr << "GL_STACK_UNDERFLOW - An attempt has been made to perform an operation that would cause the stack to underflow.";
                break;
            case GL_STACK_OVERFLOW:
                std::cerr << "GL_STACK_OVERFLOW - An attempt has been made to perform an operation that would cause a stack overflow.";
                break;
            // Add more cases for other specific OpenGL errors if needed
            default:
                std::cerr << "Unknown OpenGL error code: " << error;
        }
        std::cerr << std::endl;
    }
}

// Basic Perlin Noise implementation (simplified 2D version)
float perlinNoise(float x, float y) {
    // ... (Implement Perlin noise algorithm here - this is a simplified example) ...
    // For a complete Perlin noise implementation, refer to online resources.
    // This is a placeholder for demonstration -  it will produce very basic noise.
    float value = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxAmplitude = 0.0f;

    for (int i = 0; i < 4; ++i) { // 4 octaves
        //float sampleX = x * frequency;
        //float sampleY = y * frequency;
        //value += glm::perlin(glm::vec2(sampleX, sampleY)) * amplitude; // Using glm::perlin for simplicity
        maxAmplitude += amplitude;
        amplitude *= 0.5f; // Reduce amplitude for each octave
        frequency *= 2.0f; // Increase frequency for each octave
    }
    return value / maxAmplitude; // Normalize to [0, 1] range
}

void convert_vec3_to_float_array(const std::vector<glm::vec3>& src, float * dst) {
    // Ensure dst is the correct size (numVertices * 3) - should be handled by resize in constructor
    size_t numVertices = src.size();

    for (size_t i = 0; i < numVertices; ++i) {
        dst[i * 3 + 0] = src[i].x;
        dst[i * 3 + 1] = src[i].y;
        dst[i * 3 + 2] = src[i].z;
    }
}

void convert_float_array_to_vec3(float * src, std::vector<glm::vec3>& dst) {
size_t numVertices = dst.size(); // Assume dst has correct size already

    for (size_t i = 0; i < numVertices; ++i) {
        dst[i].x = src[i * 3 + 0];
        dst[i].y = src[i * 3 + 1];
        dst[i].z = src[i * 3 + 2];
    }
}
