/*
 * File:        Game.cpp
 * Author:      Tomas Goldmann
 * Date:        2025-03-23
 * Description: This class integrates all items into the game world.
 *
 * Copyright (c) 2025, Brno University of Technology. All rights reserved.
 * Licensed under the MIT.
 */

#include "Game.h"

Game* Game::instance = nullptr;

Game::Game() : renderer(), input(), ocean(200), boat(), camera(), terrain(150, 1.0f)  {
    instance = this;
}

Game::~Game() {
    instance = nullptr;
}

bool Game::init(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("3D Boat Simulation");

    // **Initialize GLEW AFTER creating the window:**
    if (glewInit() != GLEW_OK) {
        //std::cerr << "GLEW initialization failed!" << std::endl;
        return false;
    }



    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutKeyboardFunc(keyboardCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutTimerFunc(16, timerCallback, 0); // ~60 FPS

    glEnable(GL_DEPTH_TEST);
    renderer.init();
    //int oceanGridSize = 1000; // Default gridSize (you can change this)
    //ocean = Ocean(oceanGridSize); // Pass gridSize to constructor
    ocean.init();

    if (!boat.init("assets/models/boat.obj", "assets/models/boat.jpg")) {
        std::cerr << "Boat initialization failed!" << std::endl;
        return false;
    }
    boat.setScale(0.01f); // Example: Make the boat half its original size




    std::cerr << "Terrain init" << std::endl;

    // Initialize Terrain
    if (!terrain.init(renderer.heightMapTextureID)) {
        std::cerr << "Terrain initialization failed!" << std::endl;
        return false;
    }
    std::cerr << "Camera init" << std::endl;

    camera.init();
    std::cerr << "Input init" << std::endl;

    input.init();
    return true;
}

void Game::run() {
    glutMainLoop();
}

void Game::cleanup() {
    renderer.cleanup();
    ocean.cleanup();
    boat.cleanup();
    terrain.cleanup(); // Cleanup terrain

}

void Game::displayCallback() {
    instance->renderer.renderScene(instance->ocean, instance->boat, instance->camera, instance->terrain); // **Pass instance->terrain**
        glutSwapBuffers();
}

void Game::reshapeCallback(int width, int height) {
    instance->renderer.reshape(width, height);
    instance->camera.setAspectRatio(static_cast<float>(width) / height);
}

void Game::keyboardCallback(unsigned char key, int x, int y) {
    instance->input.handleKeyPress(key);
}

void Game::keyboardUpCallback(unsigned char key, int x, int y) {
    instance->input.handleKeyRelease(key);
}

void Game::specialCallback(int key, int x, int y) {
    instance->input.handleSpecialKeyPress(key);
}

void Game::specialUpCallback(int key, int x, int y) {
    instance->input.handleSpecialKeyRelease(key);
}

void Game::mouseCallback(int button, int state, int x, int y) {
    instance->input.handleMouseClick(button, state, x, y);
}

void Game::motionCallback(int x, int y) {
    instance->input.handleMouseMove(x, y);
}

void Game::updateGame() {
    float deltaTime = 1.0f / 60.0f; // Fixed timestep for simplicity
    instance->input.update();
    instance->boat.update(instance->input, instance->ocean, deltaTime);
    instance->camera.update(instance->input, instance->boat.getPosition()); // Camera follows boat (optional)
    instance->ocean.update(deltaTime);
}

void Game::timerCallback(int value) {
    instance->updateGame();
    glutTimerFunc(16, timerCallback, 0); // Re-register timer
    glutPostRedisplay(); // Request redraw
}

