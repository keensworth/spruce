#pragma once

#include "InputManager.h"
#include <functional>
#include "../../external/imgui/imgui_impl_sdl2.h"

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
    void addEventListener(std::function<void (SDL_Event* e)> func);
    bool quit;

private:
    InputManager inputManager;
    KeyboardState* keyboard;
    MouseState* mouse;
    SDL_Event event;
    std::vector<std::function<void (SDL_Event* e)>> m_eventListeners;


    void handleKeyPress(bool keyDown);
    void handleButtonPress(bool buttonDown);
    void handleMouseMotion();
    void handleMouseWheel();

    SprKey getSprKeyFromSDLKeycode(SDL_Keycode keycode);
    SprButton getSprButtonFromSDLButton(int button);
};
}