/*
 * File:        Camera.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: Camera
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */


#include "Camera.h"
#include <GL/freeglut.h>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera() : position(10.0f, 10.0f, 10.0f), target(0.0f, 0.0f, 0.0f), up(0.0f, 1.0f, 0.0f),
                   aspectRatio(1.0f), fov(45.0f), nearPlane(0.1f), farPlane(100.0f) {}

Camera::~Camera() {}

void Camera::init() {
    // Initial camera setup if needed
}

void Camera::update(const Input& input, const glm::vec3& boatPosition) {
    target = boatPosition;

    glm::vec3 lookDirection;
    lookDirection.x = cos(glm::radians(pitchAngle)) * cos(glm::radians(yawAngle));
    lookDirection.y = sin(glm::radians(pitchAngle));
    lookDirection.z = cos(glm::radians(pitchAngle)) * sin(glm::radians(yawAngle));
    lookDirection = glm::normalize(lookDirection);

    position = target - lookDirection * glm::vec3(10.0f); // **Update position here in update**
    position.y = glm::max(position.y, 2.0f);

    //position = boatPosition + glm::vec3(5.0f, 5.0f, 5.0f); // Offset from boat
    //position.y = glm::max(position.y, 2.0f); // Keep camera above water

    up = glm::vec3(0.0f, 1.0f, 0.0f);    

    glm::vec3 forward = glm::normalize(target - position);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    // Recalculate up to ensure orthogonality (important for gluLookAt)
    up = glm::normalize(glm::cross(right, forward));

    handleMouseInput(input, 1.0f/60.0f); // Example deltaTime (fixed 60fps for now) 
}

void Camera::lookAt() const {
    gluLookAt(position.x, position.y, position.z,
              target.x, target.y, target.z,
              up.x, up.y, up.z);
}

glm::mat4 Camera::getViewMatrix() const { // Keep `const` method
    // Recalculate camera position and up vector based on yawAngle and pitchAngle


    // **Calculate view matrix based on CURRENT position, target, up - NO MODIFICATION of members in getViewMatrix**
    return glm::lookAt(position, target, up); // Use current position, target, up
}


void Camera::handleMouseInput(const Input& input, float deltaTime) {
    if (input.isMouseButtonDown(GLUT_RIGHT_BUTTON)) { // Rotate only when right mouse button is pressed
        float mouseSensitivity = 0.1f; // Adjust sensitivity
        yawAngle   += input.getMouseDeltaX() * mouseSensitivity;
        pitchAngle -= input.getMouseDeltaY() * mouseSensitivity; // Invert vertical mouse motion

        // Clamp pitch angle to prevent camera flipping
        pitchAngle = glm::clamp(pitchAngle, -89.0f, 89.0f);

    }
}

void Camera::rotateYaw(float angle) {
    yawAngle += angle;
}

void Camera::rotatePitch(float angle) {
    pitchAngle += angle;
}
