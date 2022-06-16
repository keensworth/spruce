#pragma once

#include <vector>
#include "System.h"


namespace spr {
class SystemManager{
public:
    SystemManager();
    ~SystemManager() {}

    // add a system
    void addSystem(System& system);

    // set the render sytem
    void setRenderSystem(System& renderSystem);

    // set the audio system
    void setAudioSystem(System& audioSystem);

    // update all systems sequentially
    void update(float dt);

private:
    std::vector<System*> m_systems;
    System* m_renderSystem;
    System* m_audioSystem;

    static System m_defaultSys;
};
}