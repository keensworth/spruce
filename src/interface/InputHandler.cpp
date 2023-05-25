#include "InputHandler.h"
#include <iostream>
#include "../debug/SprLog.h"

namespace spr {
InputHandler::InputHandler(){
    keyboard = new KeyboardState();
    mouse = new MouseState();
    m_inputManager = InputManager(keyboard, mouse);
    quit = false;

    m_updatedKeys.reserve(256);
    m_updatedButtons.reserve(256);
    m_updatedMousePos = false;
    m_updatedMouseWheel = false;
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
    m_updatedKeys.push_back(key);

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
    m_updatedButtons.push_back(button);

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
    m_updatedMousePos = true;
}

void InputHandler::handleMouseWheel(){
    ivec2 scrollMotion;
    scrollMotion.x = m_event.wheel.x;
    scrollMotion.y = m_event.wheel.y;

    mouse->scrollWheelMotion = scrollMotion;
    mouse->scrollTicks = SDL_GetTicks();
    m_updatedMouseWheel = true;
}

void InputHandler::updatePreviousState(){
    // store previous key/button state from last frame's updates
    for (SprKey key : m_updatedKeys)
        keyboard->keyDownPrev[key] = keyboard->keyDown[key];
    for (SprButton button : m_updatedButtons)
        mouse->buttonDownPrev[button] = mouse->buttonDown[button];
    if (!m_updatedKeys.empty())
        m_updatedKeys.clear();
    if (!m_updatedButtons.empty())
        m_updatedButtons.clear();
    
    // store prev mouse pos/motion
    if (m_updatedMousePos){
        mouse->mousePosPrev = mouse->mousePos; // don't reset
        mouse->mouseMotionPrev = mouse->mouseMotion;
        mouse->mouseMotion = {0, 0};
        m_updatedMousePos = false;
    }

    // store prev mouse wheel motion
    if (m_updatedMouseWheel){
        mouse->scrollWheelMotionPrev = mouse->scrollWheelMotion;
        mouse->scrollWheelMotion = {0, 0};
        m_updatedMouseWheel = false;
    }
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