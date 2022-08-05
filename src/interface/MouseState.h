#pragma once

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

using namespace glm;

namespace spr {
class MouseState {
public:
    MouseState();
    ~MouseState();

    void setMousePos(ivec2 pos);
    void setMouseMovement(ivec2 delta);
    void setScrollWheelMovement(ivec2 delta);
    void setLClk(bool state);
    void setRClk(bool state);

    ivec2 getMousePos();
    ivec2 getMouseMovement();
    ivec2 getScrollWheelMovement();
    bool getLClk();
    bool getRClk();

    // mouse timing
    int timeSinceLClkUp;
    int timeSinceLClkDown;
    int timeSinceRClkUp;
    int timeSinceRClkDown;

    int timeSinceMouseMove;

    int timeSinceScrollUp;
    int timeSinceScrollDown;

private:
    // mouse state
    ivec2 mousePos;
    ivec2 mouseMovement;
    ivec2 scrollWheelMovement;
    bool lClkDown;
    bool rClkDown;

};
}