#include "InputManager.h"

namespace spr {
InputManager::InputManager(KeyboardState* keyboard, MouseState* mouse){
    m_keyboard = keyboard;
    m_mouse = mouse;
}

bool InputManager::isKeyDown(SprKey key){
    return m_keyboard->keyDown[key];
}

bool InputManager::isButtonDown(SprButton button){
    return m_mouse->buttonDown[button];
}

ivec2 InputManager::getMousePos(){
    return m_mouse->mousePos;
}

ivec2 InputManager::getMouseMotion(){
    return m_mouse->mouseMotion;
}

ivec2 InputManager::getScrollWheelMotion(){
    return m_mouse->scrollWheelMotion;
}


int InputManager::timeSinceKeyDown(SprKey key){
    int currentTime = SDL_GetTicks();
    return currentTime - m_keyboard->keyDownTicks[key];
}

int InputManager::timeSinceKeyUp(SprKey key){
    int currentTime = SDL_GetTicks();
    return currentTime - m_keyboard->keyUpTicks[key];
}

int InputManager::timeSinceButtonDown(SprButton button){
    int currentTime = SDL_GetTicks();
    return currentTime - m_mouse->buttonDownTicks[button];
}

int InputManager::timeSinceButtonUp(SprButton button){
    int currentTime = SDL_GetTicks();
    return currentTime - m_mouse->buttonUpTicks[button];
}

int InputManager::timeSinceMouseMotion(){
    int currentTime = SDL_GetTicks();
    return currentTime - m_mouse->mouseMotionTicks;
}

int InputManager::timeSinceScroll(){
    int currentTime = SDL_GetTicks();
    return currentTime - m_mouse->scrollTicks;
}
}