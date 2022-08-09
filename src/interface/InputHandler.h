#pragma once

#include <SDL2/SDL.h>
#include "InputManager.h"

namespace spr {
class InputHandler {
public:
    InputHandler();
    ~InputHandler(){
        delete keyboard;
        delete mouse;
    }
    
    void update();
    InputManager* getInputManager();

private:
    InputManager inputManager;
    KeyboardState* keyboard;
    MouseState* mouse;
    SDL_Event event;

    void handleKeyPress(bool keyDown);
    void handleButtonPress(bool buttonDown);
    void handleMouseMotion();
    void handleMouseWheel();
};
}