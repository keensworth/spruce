#include "KeyboardConfig.h"

namespace spr {
    
SprKey KeyboardConfig::getSprKeyFromSDLKeycode(SDL_Keycode keycode){
    for (int i = 0; i < KEY_COUNT; i++){
        if (keys[i] == keycode)
            return sprKeys[i].effectiveKey;
    }
    return SPR_UNKNOWN;
}

}