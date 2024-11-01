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
        m_entityId = 0;
    }
    
    ~SprECS() {}

    void update(float dt){
        // create/destroy queued entities from last frame
        entityManager.update();

        // update all systems
        systemManager.update(dt);    

        // unregister queued entities from last frame
        componentManager.update();

        // remove tracked created/destryed entites from last frame
        entityManager.cleanUp();
    }
    

    // ---------------- entity ------------------

    // create entity with components
    template<typename T, typename... Args>
    Entity createEntity(T t, Args... args){
        // get mask from components
        uint64 mask = componentManager.getMask<T,Args...>();
        
        // create entity
        Entity entity{m_entityId, mask};
        
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

        // unregister entity
        componentManager.unregisterEntity(entity);
    }

    // get a collection of entities with given templated components
    template <typename Arg, typename ...Args>
    void getEntities(std::vector<Entity>& out){
        // get mask from components
        uint64 mask = componentManager.getMask<Arg, Args...>();

        // get entities that have all components in mask
        entityManager.getEntities(mask, out);
    }

    // filter entities queued for removal/deletion for those with given components
    template <typename Arg, typename ...Args>
    void getDeletedEntities(std::vector<Entity>& out){
        // get mask from components
        uint64 mask = componentManager.getMask<Arg, Args...>();

        // get queued entities
        std::vector<Entity>& queuedEntities = entityManager.m_entitiesRemove;

        for (Entity& entity : queuedEntities){
            if (entity.components & mask){
                out.push_back(entity);
            }
        }
    }

    // filter entities queued for removal/deletion for those with given components
    template <typename Arg, typename ...Args>
    void getCreatedEntities(std::vector<Entity>& out){
        // get mask from components
        uint64 mask = componentManager.getMask<Arg, Args...>();

        // get queued entities
        std::vector<Entity>& queuedEntities = entityManager.m_entitiesAdd;

        for (Entity& entity : queuedEntities){
            if (entity.components & mask){
                out.push_back(entity);
            }
        }
    }

    // filter entities for those with a set dirty flag on given component
    template <typename Arg>
    void getDirtyEntities(std::vector<Entity>& in, std::vector<Entity>& out){
        componentManager.getDirtyEntities<Arg>(in, out);
    }

    // filter entities queued for removal/deletion for those with given component
    template <typename Arg>
    std::vector<uint32>& getDirtyEntityIds(){
        // get dirty entities
        return componentManager.getDirtyEntityIds<Arg>();        
    }

    // check if an entity's component is dirty
    template <typename Arg>
    bool isDirty(Entity& entity){
        return componentManager.isDirty<Arg>(entity);
    }


    // ---------------- component ---------------

    // add component to manager
    template <typename T>
    void createComponent(Component& component, bool enableDirtyFlagTracking = false){
        componentManager.addComponent<T>(component, enableDirtyFlagTracking);
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
    auto& get(Entity& entity){
        return componentManager.getEntityComponent<T>(entity);
    }

    template <typename T>
    auto& get(uint32 entityId){
        return componentManager.getEntityComponent<T>(entityId);
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