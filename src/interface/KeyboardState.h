#pragma once

#include "KeyboardConfig.h"

namespace spr {
class KeyboardState {
public:
    KeyboardState() {
        for (int i = 0; i < KEY_COUNT; i++){
            timeSinceKeyDown[i] = 0;
            timeSinceKeyUp[i] = 0;
            keyDown[i] = false;
        }
    }
    ~KeyboardState() {}

    int timeSinceKeyDown[KEY_COUNT];
    int timeSinceKeyUp[KEY_COUNT];
    bool keyDown[KEY_COUNT];
};
}