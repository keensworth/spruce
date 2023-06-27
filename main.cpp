#include "InputManager.h"
#include "KeyboardConfig.h"
#include "data/asset_ids.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "src/interface/SprWindow.h"
#include "src/render/SprRenderer.h"
#include "src/render/scene/Material.h"
#include "src/render/scene/SceneData.h"
#include "src/resource/ResourceTypes.h"
#include "src/resource/SprResourceManager.h"
#include "src/debug/SprLog.h"
#include <chrono>
#include <thread>
#include "glm/gtx/string_cast.hpp"
#include "util/Container.h"
#include "util/node/EntityNode.h"
#include <glm/gtx/vector_angle.hpp>
#include <chrono>

//#define USE_ECS

using namespace spr;

#ifndef USE_ECS
void handleInput(InputManager& input, vec3& pos, vec3& dir, vec3& up, vec3& right, SprWindow& window, float dt){
    if (input.isKeyDownEdge(SPR_TAB)){
        window.setRelativeMouse(!window.isRelativeMouse());
    }
    
    vec3 zAxis = {0.f, 0.f, 1.f};

    float vel = 4.0f;
    if (input.isKeyDown(SPR_LSHIFT)){
        vel *= 1.75f;
    }
    if (input.isKeyDown(SPR_LCTRL)){
        vel *= 0.75f;
    }
    if (input.isKeyDown(SPR_w)){
        pos +=  dt * vel * dir;
    }
    if (input.isKeyDown(SPR_s)){
        pos += -1.f * dt * vel * dir;
    }
    if (input.isKeyDown(SPR_a)){
        pos += -1.f * dt * vel * right;
    }
    if (input.isKeyDown(SPR_d)){
        pos +=  dt * vel * right;
    }
    if (input.isKeyDown(SPR_SPACE)){
        pos +=  dt * vel * zAxis;
    }
    
    float yaw = 0.f;
    float pitch = 0.f;
    if (window.isRelativeMouse()){
        yaw   = -(float)input.getMouseMotion().x * 0.0005f;
        pitch = -(float)input.getMouseMotion().y * 0.0005f;
    }

    // horizontal camera
    dir = normalize(rotate(angleAxis(yaw, zAxis), dir));
    right = normalize(rotate(angleAxis(yaw, zAxis), right));

    // vertical camera
    vec3 temp = rotate(angleAxis(pitch, right), dir);
    float dotZ = max(dot(temp, zAxis), dot(temp, -zAxis));
    if (dotZ >= 0.97f)
        dir = normalize(rotate(angleAxis(glm::acos(0.96999f)*sign(-dir.z), right), zAxis*sign(dir.z)));
    else 
        dir = temp;
    up = normalize(cross(right, dir));
    
}


int main() {
    // window and input
    SprWindow window = SprWindow(std::string("Spruce Test"), 1920, 1080);
    window.init();
    window.setRelativeMouse(true);
    InputManager& input = window.getInputManager();

    // renderer and assets
    SprRenderer renderer(&window);
    SprResourceManager rm;
    renderer.loadAssets(rm);

    // initial camera config
    vec3 pos = {0.f, 1.f, 0.f};
    vec3 dir = {1.f, 0.f, 0.f};
    vec3 up = {0.f, 0.f, 1.f};
    vec3 right = {0.f, -1.f, 0.f};

    // timing
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto printTime = std::chrono::high_resolution_clock::now();
    uint32 frame = 0;
    double dt = 0.f;

    // simple game loop
    while (!input.isKeyDown(spr::SPR_ESCAPE)){
        window.update();

        start = std::chrono::high_resolution_clock::now();
        
        // insert models using their ids directly
        renderer.insertModel(data::helmet, {
            .position = {0.f, 0.f, 0.f}, 
            .rotation = angleAxis((pi<float>()/2)*(frame/240.f), vec3{1.f, 0.f, 0.f}), 
            .scale = 0.2f
        });
        renderer.insertModel(data::helmet, {
            .position = {1.f, 0.f, 0.f}, 
            .rotation = angleAxis((pi<float>()/2), vec3{1.f, 0.f, 0.f}), 
            .scale = 0.2f
        });
        renderer.insertModel(data::waterbottle, {
            .position = {3.f, 0.f, 0.f}, 
            .rotation = angleAxis((pi<float>()/2), vec3{1.f, 0.f, 0.f}), 
            .scale = 3.f
        });
        renderer.insertModel(data::boomboxwithaxes, {
            .position = {-2.5f, 0.f, -1.f}, 
            .rotation = angleAxis(0.f, vec3{1.f, 0.f, 0.f}), 
            .scale = 10.f
        });
        renderer.insertModel(data::sponza, {
            .position = {0.f, 0.f, -5.f}, 
            .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
            .scale = 0.012f
        });

        
        // lights
        renderer.insertLight({.pos = {13.f, 5.6f, 3.f}, .intensity = 8.f, .range = 16.f, .color = {0.9f, 0.55f, 0.89f}});
        renderer.insertLight({.pos = {13.f, -5.6f, 3.f}, .intensity = 8.f, .range = 16.f, .color = {0.225f, 0.91f, 0.33f}});
        renderer.insertLight({.pos = {-13.f, 5.6f, 3.f}, .intensity = 8.f, .range = 16.f, .color = {0.27f, 0.88f, 0.94f}});
        renderer.insertLight({.pos = {-13.f, -5.6f, 3.f}, .intensity = 8.f, .range = 16.f, .color = {0.98f, 0.66f, 0.0f}});
        //renderer.insertLight({.pos = {5.f*glm::cos((frame/400.f)), 0.f, -1.f}, .intensity = 16.f, .range = 16.f});
        renderer.insertLight({.intensity = 4.f, .dir = glm::normalize(vec3(0.3f, 1.f, -2.f)), .type = gfx::DIRECTIONAL});

        // handle input + update camera
        handleInput(input, pos, dir, up, right, window, dt);
        renderer.updateCamera({.pos = pos, .dir = dir, .up = up});

        // render 
        renderer.render();

        // timing + window update
        stop = std::chrono::high_resolution_clock::now();
        auto printDur = (stop - printTime);
        auto dur = (stop - start);
        double fps = (double)(1000000000)/dur.count();
        dt = (double)dur.count()/(1000000000);

        // print stats
        if (printDur.count() > 1000000000){
            printTime = std::chrono::high_resolution_clock::now();
            SprLog::info("[MAIN] frame: ", frame);
            SprLog::info("[MAIN] fps: " + std::to_string(fps) );
            SprLog::info("[MAIN] dt: " + std::to_string(dt) );
            SprLog::info("[MAIN] pos: " + glm::to_string(pos) );
        }
        frame++;
    }

    return 1;
}

#else

#include "ecs/SprECS.h"

// components
class TransformC : public TypedComponent<TransformInfo>{};
class LightC : public TypedComponent<gfx::Light>{};
class CameraC : public TypedComponent<gfx::Camera>{};
class ModelC : public TypedComponent<uint32>{};

// systems
class RenderSystem : public System {
public:
    RenderSystem(SprECS* ecs, SprWindow* window, SprResourceManager* srm) : m_renderer(window){
        m_ecs = ecs;
        m_window = window;
        m_srm = srm;
        m_renderer.loadAssets(*m_srm);
    }
    ~RenderSystem(){}

    void update(float dt){
        // models
        Container<Entity> entities = m_ecs->getEntities<ModelC, TransformC>();
        for (uint32 i = 0; i < entities.getSize(); i++){
            Entity entity = entities.get(i);
            uint32 modelId = m_ecs->get<ModelC>(entity);
            TransformInfo transform = m_ecs->get<TransformC>(entity);
            m_renderer.insertModel(modelId, transform);
        }

        // lights
        Container<Entity> lights = m_ecs->getEntities<LightC>();
        for (uint32 i = 0; i < lights.getSize(); i++){
            Entity entity = lights.get(i);
            gfx::Light light = m_ecs->get<LightC>(entity);
            m_renderer.insertLight(light);
        }

        // camera(s)
        Container<Entity> cameras = m_ecs->getEntities<CameraC>();
        for (uint32 i = 0; i < cameras.getSize(); i++){
            Entity entity = cameras.get(i);
            gfx::Camera camera = m_ecs->get<CameraC>(entity);
            m_renderer.updateCamera(camera);
        }

        m_renderer.render();
    }

    SprECS* m_ecs;
    SprWindow* m_window;
    SprResourceManager* m_srm;
    SprRenderer m_renderer;
};

class InputSystem : public System {
public:
    InputSystem(SprECS* ecs, SprWindow* window){
        m_ecs = ecs;
        m_window = window;
    }
    ~InputSystem(){}

    void update(float dt){
        InputManager& input = m_window->getInputManager();

        // camera
        Container<Entity> cameras = m_ecs->getEntities<CameraC>();
        gfx::Camera& camera = m_ecs->get<CameraC>(cameras.get(0));

        if (input.isKeyDownEdge(SPR_TAB)){
            m_window->setRelativeMouse(!m_window->isRelativeMouse());
        }
    
        vec3 zAxis = {0.f, 0.f, 1.f};

        float vel = 4.0f;
        if (input.isKeyDown(SPR_LSHIFT)){
            vel *= 1.75f;
        }
        if (input.isKeyDown(SPR_LCTRL)){
            vel *= 0.75f;
        }
        if (input.isKeyDown(SPR_w)){
            camera.pos +=  dt * vel * camera.dir;
        }
        if (input.isKeyDown(SPR_s)){
            camera.pos += -1.f * dt * vel * camera.dir;
        }
        if (input.isKeyDown(SPR_a)){
            camera.pos += -1.f * dt * vel * right;
        }
        if (input.isKeyDown(SPR_d)){
            camera.pos +=  dt * vel * right;
        }
        if (input.isKeyDown(SPR_SPACE)){
            camera.pos +=  dt * vel * zAxis;
        }
        
        float yaw = 0.f;
        float pitch = 0.f;
        if (m_window->isRelativeMouse()){
            yaw   = -(float)input.getMouseMotion().x * 0.0005f;
            pitch = -(float)input.getMouseMotion().y * 0.0005f;
        }

        // horizontal camera
        camera.dir = normalize(rotate(angleAxis(yaw, zAxis), camera.dir));
        right = normalize(rotate(angleAxis(yaw, zAxis), right));

        // vertical camera
        vec3 temp = rotate(angleAxis(pitch, right), camera.dir);
        float dotZ = max(dot(temp, zAxis), dot(temp, -zAxis));
        if (dotZ >= 0.97f)
            camera.dir = normalize(rotate(angleAxis(glm::acos(0.96999f)*sign(-camera.dir.z), right), zAxis*sign(camera.dir.z)));
        else 
            camera.dir = temp;
        camera.up = normalize(cross(right, camera.dir));
    }

    vec3 right = {1.f, 0.f, 0.f};

    SprECS* m_ecs;
    SprWindow* m_window;
};

int main() {
    // window and input
    SprWindow window = SprWindow(std::string("Spruce Test"), 1920, 1080);
    window.init();
    window.setRelativeMouse(true);
    InputManager& input = window.getInputManager();

    // assets
    SprResourceManager rm;

    // -- ecs --
    SprECS ecs;

    // components
    TransformC transformC;
    LightC lightC;
    CameraC cameraC;
    ModelC modelC;
    ecs.createComponent<TransformC>(transformC);
    ecs.createComponent<LightC>(lightC);
    ecs.createComponent<CameraC>(cameraC);
    ecs.createComponent<ModelC>(modelC);

    // systems
    InputSystem inputSystem(&ecs, &window);
    RenderSystem renderSystem(&ecs, &window, &rm);
    ecs.createSystem(inputSystem);
    ecs.setRenderSystem(renderSystem);

    // timing
    auto start = std::chrono::high_resolution_clock::now();
    auto stop = std::chrono::high_resolution_clock::now();
    auto printTime = std::chrono::high_resolution_clock::now();
    uint32 frame = 0;
    double dt = 0.f;

    // scene
    {
        // models
        ecs.createEntity(
            ecs.add<ModelC>(data::helmet),
            ecs.add<TransformC>(TransformInfo{
                .position = {0.f, 0.f, 0.f}, 
                .rotation = angleAxis((pi<float>()/2)*(frame/240.f), vec3{1.f, 0.f, 0.f}), 
                .scale = 0.2f}));

        ecs.createEntity(
            ecs.add<ModelC>(data::helmet),
            ecs.add<TransformC>(TransformInfo{
                .position = {1.f, 0.f, 0.f}, 
                .rotation = angleAxis((pi<float>()/2), vec3{1.f, 0.f, 0.f}), 
                .scale = 0.2f}));

        ecs.createEntity(
            ecs.add<ModelC>(data::waterbottle),
            ecs.add<TransformC>(TransformInfo{
                .position = {3.f, 0.f, 0.f}, 
                .rotation = angleAxis(0.f, vec3{1.f, 0.f, 0.f}), 
                .scale = 3.f}));

        ecs.createEntity(
            ecs.add<ModelC>(data::boomboxwithaxes),
            ecs.add<TransformC>(TransformInfo{
                .position = {-2.5f, 0.f, -1.f}, 
                .rotation = angleAxis(pi<float>()/2.f, vec3{1.f, 0.f, 0.f}), 
                .scale = 10.f}));

        ecs.createEntity(
            ecs.add<ModelC>(data::sponza),
            ecs.add<TransformC>(TransformInfo{
                .position = {0.f, 0.f, -5.f}, 
                .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
                .scale = 0.012f}));
        
        // lights
        // ecs.createEntity(
        //     ecs.add<LightC>(gfx::Light{
        //         .pos = {3.f, 0.f, -1.f}, 
        //         .intensity = 1.f, 
        //         .range = 16.f
        //     }));

        // ecs.createEntity(
        //     ecs.add<LightC>(gfx::Light{
        //         .pos = {-3.f, 0.f, -1.f}, 
        //         .intensity = 1.f, 
        //         .range = 16.f
        //     }));

        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .intensity = 4.f, 
                .dir = glm::normalize(vec3(0.3f, 1.f, -2.f)), 
                .type = gfx::DIRECTIONAL
            }));

        // camera
        ecs.createEntity(
            ecs.add<CameraC>(gfx::Camera{
                .pos = {0.f, 1.f, 0.f},
                .dir = {0.f, 1.f, 0.f},
                .up = {0.f, 0.f, 1.f}
            }));
    }

    // simple game loop
    while (!input.isKeyDown(spr::SPR_ESCAPE)){
        start = std::chrono::high_resolution_clock::now();

        // update window
        window.update();

        // update ecs
        ecs.update(dt);

        // timing
        stop = std::chrono::high_resolution_clock::now();
        auto printDur = (stop - printTime);
        auto dur = (stop - start);
        double fps = (double)(1000000000)/dur.count();
        dt = (double)dur.count()/(1000000000);

        // print stats
        if (printDur.count() > 1000000000){
            printTime = std::chrono::high_resolution_clock::now();
            SprLog::info("[MAIN] frame: ", frame);
            SprLog::info("[MAIN] fps: " + std::to_string(fps) );
            SprLog::info("[MAIN] dt: " + std::to_string(dt) );
        }
        frame++;
    }
}

#endif