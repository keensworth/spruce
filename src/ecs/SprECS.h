#pragma once
#include "SystemManager.h"
#include "EntityManager.h"
#include "ComponentManager.h"
#include "Entity.h"

namespace spr {
class SprECS {
public:
    // ---------------- sprecs ------------------ 
    SprECS() {
        entityManager = EntityManager();
        componentManager = ComponentManager();
        systemManager = SystemManager();

        m_entityId = 0;
    }
    ~SprECS() {}

    void update(float dt){
        // create/destroy entites from last frame
        entityManager.update();

        // update all systems
        systemManager.update(dt);    
    }
    

    // ---------------- entity ------------------

    // create entity with components
    template<typename T, typename... Args>
    Entity createEntity(T t, Args... args){
        // get mask from components
        uint64 mask = componentManager.getMask<T,Args...>();
        
        // create entity
        Entity entity = Entity(m_entityId, mask);
        

        // register entity 
        entityManager.addEntity(entity);
        

        // register entity with components
        componentManager.registerEntity(entity, t, args...);
        m_entityId++;

        return entity;
    }

    // destroy entity and remove it from components and registries
    void destroyEntity(Entity& entity){
        // remove entity 
        entityManager.removeEntity(entity);

        // register entity with components
        componentManager.unregisterEntity(entity);
    }

    // get a collection of entities with given templated components
    template <typename Arg, typename ...Args>
    Container<Entity> getEntities(){
        // get mask from components
        uint64 mask = componentManager.getMask<Arg, Args...>();

        // get entities that have all components in mask
        Container<Entity> entities = entityManager.getEntities(mask);
        return entities;
    }


    // ---------------- component ---------------

    // add component to manager
    template <typename T>
    void createComponent(Component& component){
        componentManager.addComponent<T>(component);
    }

    // add new component to entity directly
    template <typename T>
    void add(Entity& entity, auto data){
        entityManager.removeEntity(entity);
        uint64 mask = componentManager.getMask<T>();
        entity.addComponents(mask);
        entityManager.addEntity(entity);
        componentManager.addComponentEntityData<T>(entity, data);
    }

    // remove component from entity directly
    template <typename T>
    void remove(Entity& entity){
        entityManager.removeEntity(entity);
        uint64 mask = componentManager.getMask<T>();
        entity.removeComponents(mask);
        entityManager.addEntity(entity);
        componentManager.removeComponentEntity<T>(entity);
    }

    // add component
    template <typename T>
    T* add(auto data){
        return componentManager.addComponentData<T>(data);
    }

    // set entity's component
    template <typename T>
    void set(Entity& entity, auto data){
        componentManager.setEntityComponent<T>(entity, data);
    }

    // get entity's component
    template <typename T>
    auto get(Entity& entity){
        return componentManager.getEntityComponent<T>(entity);
    }


    // ---------------- system ------------------

    // create a system
    void createSystem(System& system){
        systemManager.addSystem(system);
    }

    void setRenderSystem(System& system){
        systemManager.setRenderSystem(system);
    }

    void setAudioSystem(System& system){
        systemManager.setAudioSystem(system);
    }

private:
    // ecs managers
    EntityManager entityManager;
    ComponentManager componentManager;
    SystemManager systemManager;

    uint32 m_entityId;
};
}