#pragma once

#include <SDL2/SDL.h>
#include "KeyboardState.h"
#include "MouseState.h"

namespace spr {
class InputManager {
public:
    InputManager();
    InputManager(KeyboardState* keyboard, MouseState* mouse);
    ~InputManager(){}

    bool isKeyDown(SprKey key);
    bool isButtonDown(SprButton key);
    ivec2 getMousePos();
    ivec2 getMouseMovement();
    ivec2 getScrollWheelMovement();

    int timeSinceKeyDown(SprKey key);
    int timeSinceKeyUp(SprKey key);
    int timeSinceButtonDown(SprButton button);
    int timeSinceButtonUp(SprButton button);
    int timeSinceMouseMove();
    int timeSinceScrollUp();
    int timeSinceScrollDown();
    
private:
    KeyboardState* keyboard;
    MouseState* mouse;
};
}