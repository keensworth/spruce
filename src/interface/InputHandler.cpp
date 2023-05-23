#include "InputHandler.h"
#include <iostream>
#include "../debug/SprLog.h"

namespace spr {
InputHandler::InputHandler(){
    keyboard = new KeyboardState();
    mouse = new MouseState();
    inputManager = InputManager(keyboard, mouse);
    quit = false;
}

InputManager& InputHandler::getInputManager(){
    return inputManager;
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
    if (event.key.repeat == 1)
        return;

    SDL_Keycode keycode = event.key.keysym.sym;
    SprKey key = getSprKeyFromSDLKeycode(keycode);
    keyboard->keyDown[key] = keyDown;
    if (keyDown)
        keyboard->keyDownTicks[key] = SDL_GetTicks();
    else 
        keyboard->keyUpTicks[key] = SDL_GetTicks();
}

void InputHandler::handleButtonPress(bool buttonDown){
    int button = event.button.button;
    SprButton sprButton = getSprButtonFromSDLButton(button);
    mouse->buttonDown[sprButton] = buttonDown;
    if (buttonDown)
        mouse->buttonDownTicks[sprButton] = SDL_GetTicks();
    else 
        mouse->buttonUpTicks[sprButton] = SDL_GetTicks();
}

void InputHandler::handleMouseMotion(){
    ivec2 mousePos;
    mousePos.x = event.motion.x;
    mousePos.y = event.motion.y;

    ivec2 mouseMotion;
    mouseMotion.x = event.motion.xrel;
    mouseMotion.y = event.motion.yrel;

    mouse->mousePos = mousePos;
    mouse->mouseMotion = mouseMotion;
    mouse->mouseMotionTicks = SDL_GetTicks();
    //std::cout << "MPos: " << mousePos.x << ", " << mousePos.y << std::endl;
}

void InputHandler::handleMouseWheel(){
    ivec2 scrollMotion;
    scrollMotion.x = event.wheel.x;
    scrollMotion.y = event.wheel.y;

    mouse->scrollWheelMotion = scrollMotion;
    mouse->scrollTicks = SDL_GetTicks();
}

void InputHandler::update(){
    mouse->mouseMotion = {0, 0};
    
    while (SDL_PollEvent(&event)){
        for (auto listener : m_eventListeners){
            listener(&event);
        }
        switch( event.type ){
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
    event = SDL_Event();  
}
}