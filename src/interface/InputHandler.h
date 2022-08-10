#pragma once

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
    InputManager& getInputManager();
    bool quit;

private:
    InputManager inputManager;
    KeyboardState* keyboard;
    MouseState* mouse;
    SDL_Event event;

    void handleKeyPress(bool keyDown);
    void handleButtonPress(bool buttonDown);
    void handleMouseMotion();
    void handleMouseWheel();

    SprKey getSprKeyFromSDLKeycode(SDL_Keycode keycode);
    SprButton getSprButtonFromSDLButton(int button);
};
}