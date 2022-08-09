#pragma once

#include "KeyboardConfig.h"

namespace spr {
class KeyboardState {
public:
    KeyboardState() {
        for (int i = 0; i < KEY_COUNT; i++){
            keyUpTicks[i] = 0;
            keyDownTicks[i] = 0;
            keyDown[i] = false;
        }
    }
    ~KeyboardState() {}

    int keyDownTicks[KEY_COUNT];
    int keyUpTicks[KEY_COUNT];
    bool keyDown[KEY_COUNT];
};
}