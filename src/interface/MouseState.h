#pragma once

#include <glm/glm.hpp>
#include "MouseConfig.h"

using namespace glm;

namespace spr {
class MouseState {
public:
    MouseState() {
        mousePos = ivec2(0,0);
        mouseMovement = ivec2(0,0);
        scrollWheelMovement = ivec2(0,0);

        buttonDownTicks = 0;
        buttonUpTicks = 0;
        mouseMoveTicks = 0;
        scrollUpTicks = 0;
        scrollDownTicks = 0;

        for (int i = 0; i < BUTTON_COUNT; i++){
            buttonDown[i] = false;
        }
    }

    ~MouseState() {}

    // mouse state
    ivec2 mousePos;
    ivec2 mouseMovement;
    ivec2 scrollWheelMovement;
    bool buttonDown[BUTTON_COUNT];

    // mouse timing
    int buttonDownTicks;
    int buttonUpTicks;
    int mouseMoveTicks;
    int scrollUpTicks;
    int scrollDownTicks;
};
}