/*
 * File:        Shader.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: Shader controll
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */

#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

Shader::Shader() : vertexShaderID(0), fragmentShaderID(0), programID(0) {}

Shader::~Shader() {
    cleanup();
}


bool Shader::loadShader(const char* vertexShaderPath, const char* fragmentShaderPath) {
    // 1. Compile vertex and fragment shaders
    if (!compileShader(vertexShaderID, GL_VERTEX_SHADER, vertexShaderPath)) return false;
    if (!compileShader(fragmentShaderID, GL_FRAGMENT_SHADER, fragmentShaderPath)) return false;

    // 2. Link shader program
    if (!linkShaderProgram()) return false;

    return true; // Shader program loaded and linked successfully
}


bool Shader::compileShader(GLuint& shaderID, GLenum shaderType, const char* shaderPath) {
    shaderID = glCreateShader(shaderType);
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit); // Enable exception throwing
    try {
        shaderFile.open(shaderPath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << shaderPath << std::endl;
        return false;
    }
    const char* shaderCodeCStr = shaderCode.c_str();
    glShaderSource(shaderID, 1, &shaderCodeCStr, NULL);
    glCompileShader(shaderID);

    // Check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shaderID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED: " << shaderPath << "\n" << infoLog << std::endl;
        return false;
    }
    return true;
}


bool Shader::linkShaderProgram() {
    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check for linking errors
    int success;
    char infoLog[512];
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(programID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return false;
    }

    glDeleteShader(vertexShaderID);   // Delete shaders as they are linked into program now and no longer necessary
    glDeleteShader(fragmentShaderID); // Shader objects are deleted after linking
    vertexShaderID = 0;
    fragmentShaderID = 0;

    return true;
}


void Shader::use() {
    glUseProgram(programID);
}

void Shader::unuse() {
    glUseProgram(0); // Unuse shader program (use fixed-function pipeline)
}

void Shader::cleanup() {
    if (programID != 0) {
        glDeleteProgram(programID);
        programID = 0;
    }
}


void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(programID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
     glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}