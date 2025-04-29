// Game.h
#ifndef GAME_H
#define GAME_H
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Renderer.h"
#include "Input.h"
#include "Ocean.h"
#include "Boat.h"
#include "Camera.h"
#include <cstdio>
#include <iostream>
#include "Terrain.h" // Include Terrain header

class Game {
public:
    Game();
    ~Game();

    bool init(int argc, char** argv);
    void run();
    void cleanup();

private:
    Renderer renderer;
    Input input;
    Ocean ocean;
    Boat boat;
    Camera camera;
    Terrain terrain; // Add Terrain member


    static void displayCallback();
    static void reshapeCallback(int width, int height);
    static void keyboardCallback(unsigned char key, int x, int y);
    static void keyboardUpCallback(unsigned char key, int x, int y);
    static void specialCallback(int key, int x, int y);
    static void specialUpCallback(int key, int x, int y);
    static void mouseCallback(int button, int state, int x, int y);
    static void motionCallback(int x, int y);
    static void timerCallback(int value);
    static void updateGame();

    static Game* instance; // Singleton for callbacks to access game instance
};

#endif // GAME_H