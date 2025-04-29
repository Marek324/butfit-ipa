// Camera.h
#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include "Input.h" // Optional Shader class

class Camera {
public:
    Camera();
    ~Camera();

    void init();
    void update(const Input& input, const glm::vec3& boatPosition);
    void lookAt() const;

    void setAspectRatio(float ratio) { aspectRatio = ratio; }
    glm::vec3 getPosition() const { return position; } // Public getter for position
    glm::mat4 getViewMatrix() const; // **Declare getViewMatrix() method**

        // New: Camera Rotation Control
        void handleMouseInput(const Input& input, float deltaTime);
        void rotateYaw(float angle);
        void rotatePitch(float angle);
private:
    glm::vec3 position;
    glm::vec3 target; // Point to look at
    glm::vec3 up;
    float aspectRatio;
    float fov;
    float nearPlane;
    float farPlane;


    // New: Camera Rotation State
    float yawAngle=0.0f;
    float pitchAngle=0.0f;
    float rotationSpeed;
};

#endif // CAMERA_H