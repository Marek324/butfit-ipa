/*
 * File:        Input.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: This class defines the game inputs.
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */


#include "Input.h"
#include <GL/freeglut.h>
#include <cstring>
Input::Input() {
    memset(mouseButtonsDown, false, sizeof(mouseButtonsDown)); // Initialize mouse button states to false
    mouseX = mouseY = lastMouseX = lastMouseY = 0;
    mouseDeltaX = mouseDeltaY = 0;
}
Input::~Input() {}

void Input::init() {}
void Input::update() {
    // Calculate mouse delta
    mouseDeltaX = mouseX - lastMouseX;
    mouseDeltaY = mouseY - lastMouseY;

    // Update last mouse positions for next frame
    lastMouseX = mouseX;
    lastMouseY = mouseY;
}
void Input::handleKeyPress(unsigned char key) {
    keysDown.insert(key);
}

void Input::handleKeyRelease(unsigned char key) {
    keysDown.erase(key);
}

void Input::handleSpecialKeyPress(int key) {
    specialKeysDown.insert(key);
}

void Input::handleSpecialKeyRelease(int key) {
    specialKeysDown.erase(key);
}

void Input::handleMouseClick(int button, int state, int x, int y) {
    if (button >= 0 && button < 5) { // Check button index is within range
        mouseButtonsDown[button] = (state == GLUT_DOWN);
    }
}

void Input::handleMouseMove(int x, int y) {
    mouseX = x;
    mouseY = y;
}

bool Input::isKeyDown(unsigned char key) const {
    return keysDown.count(key);
}

bool Input::isSpecialKeyDown(int key) const {
    return specialKeysDown.count(key);
}