#pragma once

#include "InputManager.h"
#include <functional>

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
    InputManager m_inputManager;
    KeyboardState* keyboard;
    MouseState* mouse;

    SDL_Event m_event;
    std::vector<std::function<void (SDL_Event* e)>> m_eventListeners;

    std::vector<SprKey> m_updatedKeys;
    std::vector<SprButton> m_updatedButtons;
    bool m_updatedMousePos;
    bool m_updatedMouseWheel;

    void updatePreviousState();
    void handleKeyPress(bool keyDown);
    void handleButtonPress(bool buttonDown);
    void handleMouseMotion();
    void handleMouseWheel();

    SprKey getSprKeyFromSDLKeycode(SDL_Keycode keycode);
    SprButton getSprButtonFromSDLButton(int button);
};
}