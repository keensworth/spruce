#pragma once

#include "KeyboardState.h"
#include "MouseState.h"

namespace spr {
class InputManager {
public:
    InputManager() {}
    InputManager(KeyboardState* keyboard, MouseState* mouse);
    ~InputManager(){}

    bool isKeyDown(SprKey key);
    bool isButtonDown(SprButton button);
    ivec2 getMousePos();
    ivec2 getMouseMotion();
    ivec2 getScrollWheelMotion();

    int timeSinceKeyDown(SprKey key);
    int timeSinceKeyUp(SprKey key);
    int timeSinceButtonDown(SprButton button);
    int timeSinceButtonUp(SprButton button);
    int timeSinceMouseMotion();
    int timeSinceScroll();
    
private:
    KeyboardState* m_keyboard;
    MouseState* m_mouse;
};
}