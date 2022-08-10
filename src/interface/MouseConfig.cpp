#include "MouseConfig.h"

namespace spr {

SprButton MouseConfig::getSprButtonFromSDLButton(int button){
    for (int i = 0; i < BUTTON_COUNT; i++){
        if (buttons[i] == button)
            return sprButtons[i].effectiveButton;
    }
    return SPR_BUTTON_UNKNOWN;
}

}