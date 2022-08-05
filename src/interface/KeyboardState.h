#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

enum SprKey {
    test
};

namespace spr {
class KeyboardState {
public:
    void setKeyDown(SprKey key);
    void setKeyUp(SprKey key);

    bool isKeyDown(SprKey key);

    int timeSinceKeyDown[322];
    int timeSinceKeyUp[322];
private:
    bool keyDown[322];
};
}