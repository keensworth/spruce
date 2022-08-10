#pragma once
#define SDL_MAIN_HANDLED

#include <SDL2/SDL.h>

#define BUTTON_COUNT 6

namespace spr {
static const int buttons[BUTTON_COUNT] = {
    -1,
    SDL_BUTTON_LEFT,
    SDL_BUTTON_MIDDLE,
    SDL_BUTTON_RIGHT,
    SDL_BUTTON_X1,
    SDL_BUTTON_X2
};

typedef enum {
    SPR_BUTTON_UNKNOWN,
    SPR_BUTTON_LEFT,
    SPR_BUTTON_MIDDLE,
    SPR_BUTTON_RIGHT,
    SPR_BUTTON_X1,
    SPR_BUTTON_X2
} SprButton;

typedef struct {
    SprButton defaultButton;
    SprButton effectiveButton;
} SprButtonBind;

class MouseConfig{
public:
    MouseConfig(){}
    ~MouseConfig(){}
    SprButton getSprButtonFromSDLButton(int button);
public:
SprButtonBind sprButtons[BUTTON_COUNT] = {
    {SPR_BUTTON_UNKNOWN, SPR_BUTTON_UNKNOWN},
    {SPR_BUTTON_LEFT, SPR_BUTTON_LEFT},
    {SPR_BUTTON_MIDDLE, SPR_BUTTON_MIDDLE},
    {SPR_BUTTON_RIGHT, SPR_BUTTON_RIGHT},
    {SPR_BUTTON_X1, SPR_BUTTON_X1},
    {SPR_BUTTON_X2, SPR_BUTTON_X2}
};
};
}