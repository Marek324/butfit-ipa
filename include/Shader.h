#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <GL/glew.h> // Include GLEW for OpenGL types
#include <glm/glm.hpp> // **ADD THIS LINE - Include GLM header!**
#include <glm/gtc/type_ptr.hpp>



class Shader {
public:
    Shader(); // Constructor
    ~Shader(); // Destructor

    bool loadShader(const char* vertexShaderPath, const char* fragmentShaderPath); // Load and compile shaders from files
    bool isLoaded() const { return programID != 0; } // Check if shader program is loaded

    void use();       // Use (activate) the shader program
    void unuse();     // Unuse (deactivate) the shader program
    void cleanup();   // Release shader resources

    // Uniform setting functions (add more as needed for different uniform types)
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;

private:
    GLuint vertexShaderID;   // Vertex shader object ID
    GLuint fragmentShaderID; // Fragment shader object ID
    GLuint programID;        // Shader program ID

    bool compileShader(GLuint& shaderID, GLenum shaderType, const char* shaderPath);
    bool linkShaderProgram();
};

#endif // SHADER_H