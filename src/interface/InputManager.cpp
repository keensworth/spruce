#include "InputManager.h"
#include "debug/SprLog.h"

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

bool InputManager::isKeyUp(SprKey key){
    return !m_keyboard->keyDown[key];
}

bool InputManager::isButtonUp(SprButton button){
    return !m_mouse->buttonDown[button];
}

bool InputManager::isKeyDownEdge(SprKey key){
    return m_keyboard->keyDown[key] && !m_keyboard->keyDownPrev[key];
}

bool InputManager::isButtonDownEdge(SprButton button){
    return m_mouse->buttonDown[button] && !m_mouse->buttonDownPrev[button];
}

bool InputManager::isKeyUpEdge(SprKey key){
    return !m_keyboard->keyDown[key] && m_keyboard->keyDownPrev[key];
}

bool InputManager::isButtonUpEdge(SprButton button){
    return !m_mouse->buttonDown[button] && m_mouse->buttonDownPrev[button];
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


uint32 InputManager::timeSinceKeyDown(SprKey key){
    uint32 currentTime = SDL_GetTicks();
    return currentTime - m_keyboard->keyDownTicks[key];
}

uint32 InputManager::timeSinceKeyUp(SprKey key){
    uint32 currentTime = SDL_GetTicks();
    return currentTime - m_keyboard->keyUpTicks[key];
}

uint32 InputManager::timeSinceButtonDown(SprButton button){
    uint32 currentTime = SDL_GetTicks();
    return currentTime - m_mouse->buttonDownTicks[button];
}

uint32 InputManager::timeSinceButtonUp(SprButton button){
    uint32 currentTime = SDL_GetTicks();
    return currentTime - m_mouse->buttonUpTicks[button];
}

uint32 InputManager::timeSinceMouseMotion(){
    uint32 currentTime = SDL_GetTicks();
    return currentTime - m_mouse->mouseMotionTicks;
}

uint32 InputManager::timeSinceScroll(){
    uint32 currentTime = SDL_GetTicks();
    return currentTime - m_mouse->scrollTicks;
}
}