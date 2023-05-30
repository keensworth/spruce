#include "SystemManager.h"

namespace spr {
    
SystemManager::SystemManager() {}

void SystemManager::addSystem(System& system){
    m_systems.push_back(&system);
}

void SystemManager::setRenderSystem(System& renderSystem){
    m_renderSystem = &renderSystem;
}

void SystemManager::setAudioSystem(System& audioSystem){
    m_audioSystem = &audioSystem;
}

void SystemManager::update(float dt){
    // update systems
    for(System* system : m_systems){
        system->update(dt);
    }

    // update render system
    m_renderSystem->update(dt);

    // // update audio system
    // m_audioSystem->update(dt);
    
}

} // namespace spr
