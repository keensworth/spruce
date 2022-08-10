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

    // mouse timing
    int buttonDownTicks[BUTTON_COUNT];
    int buttonUpTicks[BUTTON_COUNT];
    int mouseMotionTicks;
    int scrollTicks;

    // mouse config
    MouseConfig config;
};
}