// Input.h
#ifndef INPUT_H
#define INPUT_H

#include <set>

class Input {
public:
    Input();
    ~Input();

    void init();
    void update();

    void handleKeyPress(unsigned char key);
    void handleKeyRelease(unsigned char key);
    void handleSpecialKeyPress(int key);
    void handleSpecialKeyRelease(int key);
    void handleMouseClick(int button, int state, int x, int y);
    void handleMouseMove(int x, int y);

    bool isKeyDown(unsigned char key) const;
    bool isSpecialKeyDown(int key) const;
    // Mouse input methods if needed

    // New: Mouse Input Methods
    bool isMouseButtonDown(int button) const { return mouseButtonsDown[button]; }
    int getMouseX() const { return mouseX; }
    int getMouseY() const { return mouseY; }
    int getMouseDeltaX() const { return mouseDeltaX; }
    int getMouseDeltaY() const { return mouseDeltaY; }


private:
    std::set<unsigned char> keysDown;
    std::set<int> specialKeysDown;
    // Mouse state variables if needed

        bool mouseButtonsDown[5]; // Up to 5 mouse buttons (GLUT_LEFT_BUTTON, etc.)
    int mouseX, mouseY;        // Current mouse position
    int lastMouseX, lastMouseY; // Last frame's mouse position
    int mouseDeltaX, mouseDeltaY; // Mouse movement delta since last frame
};

#endif // INPUT_H