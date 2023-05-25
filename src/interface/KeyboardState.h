#pragma once

#include "KeyboardConfig.h"
#include "spruce_core.h"

namespace spr {
class KeyboardState {
public:
    KeyboardState() {
        for (int i = 0; i < KEY_COUNT; i++){
            keyUpTicks[i] = 0;
            keyDownTicks[i] = 0;
            keyDown[i] = false;
            keyDownPrev[i] = false;
        }
        
        config = KeyboardConfig();
    }

    ~KeyboardState() {}

    // keyboard state
    bool keyDown[KEY_COUNT];
    bool keyDownPrev[KEY_COUNT];

    // keyboard timing
    uint32 keyDownTicks[KEY_COUNT];
    uint32 keyUpTicks[KEY_COUNT];

    // keyboard config
    KeyboardConfig config;
};
}