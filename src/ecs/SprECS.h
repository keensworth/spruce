#pragma once

#include "EntityManager.h"
#include "SystemManager.h"
#include "ComponentManager.h"
#include "System.h"
#include "Component.h"
#include "Entity.h"

namespace spr {
class SprECS {
public:
    // ---------------- sprecs ------------------ 
    SprECS();
    ~SprECS() {}

    void update(float dt);
    

    // ---------------- entity ------------------
    void createEntity(std::vector<Component*> components);
    void destroyEntity(Entity& entity);
    template <typename Arg, typename ...Args>
    Container<Entity> getEntities();


    // ---------------- component ---------------
    // add component to manager
    void createComponent(Component& component);

    // add new component to entity directly
    template <typename T>
    void add(Entity entity, auto data);

    // remove component from entity directly
    template <typename T>
    void remove(Entity entity);

    // add component
    template <typename T>
    void add(auto data);

    // set entity's component
    template <typename T>
    void set(Entity entity, auto data);

    // get entity's component
    template <typename T>
    auto get(Entity entity);


    // ---------------- system ------------------
    void createSystem(System& system);
    void setRenderSystem(System& system);
    void setAudioSystem(System& system);

private:
    // ecs managers
    EntityManager entityManager;
    ComponentManager componentManager;
    SystemManager systemManager;

    int m_entityId;
};
}