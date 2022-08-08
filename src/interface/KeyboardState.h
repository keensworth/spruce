#pragma once

#include "KeyboardConfig.h"

namespace spr {
class KeyboardState {
public:
    void setKeyDown(SprKey key);
    void setKeyUp(SprKey key);

    bool isKeyDown(SprKey key);

    int timeSinceKeyDown[KEY_COUNT];
    int timeSinceKeyUp[KEY_COUNT];
private:
    bool keyDown[KEY_COUNT];
};
}