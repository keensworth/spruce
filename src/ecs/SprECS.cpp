#include "SprECS.h"

namespace spr {
// -------------------- sprecs ----------------------
SprECS::SprECS(){
    entityManager = EntityManager();
    componentManager = ComponentManager();
    systemManager = SystemManager();

    m_entityId = 0;
}

void SprECS::update(float dt){
    // create/destroy entites from last frame
    entityManager.update();

    // update all systems
    systemManager.update(dt);    
}


// -------------------- entity ----------------------
void SprECS::createEntity(std::vector<Component&> components){
    // get mask from components
    long mask = componentManager.getMask(components);
    
    // create entity
    Entity entity = Entity(m_entityId, mask);

    // register entity 
    entityManager.addEntity(entity);

    // register entity with components
    componentManager.registerEntity(entity, components);
    m_entityId++;
}

void SprECS::destroyEntity(Entity& entity){
    // remove entity 
    entityManager.removeEntity(entity);

    // register entity with components
    componentManager.unregisterEntity(entity);
}

template <typename Arg, typename ...Args>
Container<Entity> SprECS::getEntities(){
    // get mask from components
    long mask = componentManager.getMask<Arg, Args>();

    // get entities that have all components in mask
    Container<Entity> entities = entityManager.getEntities(mask);
    return entities;
}


// -------------------- component -------------------
void SprECS::createComponent(Component& component){
    componentManager.addComponent(component);
}

template <typename T>
void SprECS::add(Entity entity, auto data){
    componentManager.addComponentEntityData<T>(entity, data);
}

template <typename T>
void SprECS::remove(Entity entity){
    componentManager.removeComponentEntity<T>(entity);
}

template <typename T>
void SprECS::add(auto data){
    componentManager.addComponentData<T>(data);
}

template <typename T>
void SprECS::set(Entity entity, auto data){
    componentManager.setEntityComponent<T>(entity, data);
}

template <typename T>
auto SprECS::get(Entity entity){
    return componentManager.getEntityComponent<T>(entity);
}


// -------------------- system ----------------------
void SprECS::createSystem(System& system){
    systemManager.addSystem(system);
}

void SprECS::setRenderSystem(System& system){
    systemManager.setRenderSystem(system);
}

void SprECS::setAudioSystem(System& system){
    systemManager.setAudioSystem(system);
}

}