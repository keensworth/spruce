#pragma once

#include <glm/glm.hpp>
#include "MouseConfig.h"

using namespace glm;

namespace spr {
class MouseState {
public:
    MouseState() {
        mousePos = ivec2(0,0);
        mouseMotion = ivec2(0,0);
        scrollWheelMotion = ivec2(0,0);

        mouseMotionTicks = 0;
        scrollTicks = 0;

        for (int i = 0; i < BUTTON_COUNT; i++){
            buttonDown[i] = false;
            buttonDownPrev[i] = false;
            buttonDownTicks[i] = 0;
            buttonUpTicks[i] = 0;
        }

        config = MouseConfig();
    }

    ~MouseState() {}

    // mouse state
    ivec2 mousePos;
    ivec2 mouseMotion;
    ivec2 scrollWheelMotion;
    bool buttonDown[BUTTON_COUNT];
    // (prev)
    ivec2 mousePosPrev;
    ivec2 mouseMotionPrev;
    ivec2 scrollWheelMotionPrev;
    bool buttonDownPrev[BUTTON_COUNT];

    // mouse timing
    uint32 buttonDownTicks[BUTTON_COUNT];
    uint32 buttonUpTicks[BUTTON_COUNT];
    uint32 mouseMotionTicks;
    uint32 scrollTicks;

    // mouse config
    MouseConfig config;
};
}