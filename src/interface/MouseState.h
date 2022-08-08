#pragma once

#include <glm/glm.hpp>
#include "MouseConfig.h"

using namespace glm;

namespace spr {
class MouseState {
public:
    MouseState();
    ~MouseState();

    void setMousePos(ivec2 pos);
    void setMouseMovement(ivec2 delta);
    void setScrollWheelMovement(ivec2 delta);

    void setButtonDown(SprButton key);
    void setButtonUp(SprButton key);

    bool isButtonDown(SprButton key);
    ivec2 getMousePos();
    ivec2 getMouseMovement();
    ivec2 getScrollWheelMovement();


    // mouse timing
    int timeSinceButtonDown[BUTTON_COUNT];
    int timeSinceButtonUp[BUTTON_COUNT];

    int timeSinceMouseMove;

    int timeSinceScrollUp;
    int timeSinceScrollDown;

private:
    // mouse state
    ivec2 mousePos;
    ivec2 mouseMovement;
    ivec2 scrollWheelMovement;
    bool buttonDown[BUTTON_COUNT];
};
}