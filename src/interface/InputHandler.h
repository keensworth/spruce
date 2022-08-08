#pragma once

#include <SDL2/SDL.h>

namespace spr {
class InputHandler {
public:
    void update();
private:
    InputManager inputManager;
};
}