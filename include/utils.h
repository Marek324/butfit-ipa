#ifndef UTILS_H
#define UTILS_H

#include <GL/glew.h>
#include <iostream>
#include <glm/glm.hpp>
#include <x86intrin.h>
#include <vector>

// Helper function to check for OpenGL errors and print a message
void checkGLError(const char* operation);
float perlinNoise(float x, float y); // Placeholder declaration
void convert_vec3_to_float_array(const std::vector<glm::vec3>& src, float * dst);
void convert_float_array_to_vec3(float * src, std::vector<glm::vec3>& dst);
uint64_t rdtsc();
#endif // UTILS_H