#pragma once

#include "KeyboardState.h"
#include "MouseState.h"

namespace spr {
class InputManager {
public:
    InputManager() {}
    InputManager(KeyboardState* keyboard, MouseState* mouse);
    ~InputManager(){}

    // check if key/button is down (repeats included)
    bool isKeyDown(SprKey key);
    bool isButtonDown(SprButton button);

    // check if key/button is up (repeats included)
    bool isKeyUp(SprKey key);
    bool isButtonUp(SprButton button);

    // check if key/button is on down edge (up->down)
    bool isKeyDownEdge(SprKey key);
    bool isButtonDownEdge(SprButton button);

    // check if key/button is on up edge (down->up)
    bool isKeyUpEdge(SprKey key);
    bool isButtonUpEdge(SprButton button);

    // mouse position/motions
    ivec2 getMousePos();
    ivec2 getMouseMotion();
    ivec2 getScrollWheelMotion();

    // timings
    uint32 timeSinceKeyDown(SprKey key);
    uint32 timeSinceKeyUp(SprKey key);
    uint32 timeSinceButtonDown(SprButton button);
    uint32 timeSinceButtonUp(SprButton button);
    uint32 timeSinceMouseMotion();
    uint32 timeSinceScroll();
    
private:
    KeyboardState* m_keyboard;
    MouseState* m_mouse;
};
}