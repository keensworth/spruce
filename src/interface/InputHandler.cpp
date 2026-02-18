#include "InputHandler.h"
#include <iostream>
#include "../debug/SprLog.h"
#include "KeyboardConfig.h"
#include "MouseConfig.h"

namespace spr {
InputHandler::InputHandler(){
    keyboard = new KeyboardState();
    mouse = new MouseState();
    m_inputManager = InputManager(keyboard, mouse);
    quit = false;
}

InputManager& InputHandler::getInputManager(){
    return m_inputManager;
}

SprKey InputHandler::getSprKeyFromSDLKeycode(SDL_Keycode keycode){
    return keyboard->config.getSprKeyFromSDLKeycode(keycode);
}

SprButton InputHandler::getSprButtonFromSDLButton(int button){
    return mouse->config.getSprButtonFromSDLButton(button);
}

void InputHandler::addEventListener(std::function<void (SDL_Event* e)> func){
    m_eventListeners.push_back(func);
}

void InputHandler::handleKeyPress(bool keyDown){
    if (m_event.key.repeat != 0)
        return;
    
    SDL_Keycode keycode = m_event.key.keysym.sym;
    SprKey key = getSprKeyFromSDLKeycode(keycode);

    keyboard->keyDown[key] = keyDown;

    if (keyDown)
        keyboard->keyDownTicks[key] = SDL_GetTicks();
    else 
        keyboard->keyUpTicks[key] = SDL_GetTicks();
}

void InputHandler::handleButtonPress(bool buttonDown){
    uint32 buttonCode = m_event.button.button;
    SprButton button = getSprButtonFromSDLButton(buttonCode);

    if (mouse->buttonDownPrev[button] == buttonDown)
        return;

    mouse->buttonDown[button] = buttonDown;

    if (buttonDown)
        mouse->buttonDownTicks[button] = SDL_GetTicks();
    else 
        mouse->buttonUpTicks[button] = SDL_GetTicks();
}

void InputHandler::handleMouseMotion(){
    mouse->mousePos.x = m_event.motion.x;
    mouse->mousePos.y = m_event.motion.y;
    
    mouse->mouseMotion.x += m_event.motion.xrel;
    mouse->mouseMotion.y += m_event.motion.yrel;

    mouse->mouseMotionTicks = SDL_GetTicks();
}

void InputHandler::handleMouseWheel(){
    ivec2 scrollMotion;
    scrollMotion.x = m_event.wheel.x;
    scrollMotion.y = m_event.wheel.y;

    mouse->scrollWheelMotion = scrollMotion;
    mouse->scrollTicks = SDL_GetTicks();
}

void InputHandler::updatePreviousState(){
    std::copy(keyboard->keyDown, keyboard->keyDown + KEY_COUNT, keyboard->keyDownPrev);
    std::copy(mouse->buttonDown, mouse->buttonDown + BUTTON_COUNT, mouse->buttonDownPrev);

    mouse->mousePosPrev = mouse->mousePos;
    mouse->mouseMotionPrev = mouse->mouseMotion;
    mouse->mouseMotion = {0, 0};

    mouse->scrollWheelMotionPrev = mouse->scrollWheelMotion;
    mouse->scrollWheelMotion = {0, 0};
}

void InputHandler::update(){
    updatePreviousState();
    
    // poll events
    while (SDL_PollEvent(&m_event)){
        for (auto listener : m_eventListeners){
            listener(&m_event);
        }
        switch( m_event.type ){
            // handle keyboard input
            case SDL_KEYDOWN:
                handleKeyPress(true);
                break;
            case SDL_KEYUP:
                handleKeyPress(false);
                break;

            // handle mouse button input
            case SDL_MOUSEBUTTONDOWN:
                handleButtonPress(true);
                break;
            case SDL_MOUSEBUTTONUP:
                handleButtonPress(false);
                break;

            // handle mouse wheel input
            case SDL_MOUSEWHEEL:
                handleMouseWheel();
                break;
            
            // handle mouse motion
            case SDL_MOUSEMOTION:
                handleMouseMotion();
                break;

            // handle quit
            case SDL_QUIT:
                quit = true;
                break;

            default:
                break;
        }   
    } 
    m_event = SDL_Event();  
}
}