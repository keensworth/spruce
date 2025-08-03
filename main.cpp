#include "interface/SprWindow.h"
#include "render/SprRenderer.h"
#include "resource/SprResourceManager.h"
#include "debug/SprLog.h"
#include "ecs/SprECS.h"
#include "util/Timer.h"

using namespace spr;

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
        
        SprLog::info({{"[RenderSystem] ", color::GRADIENT20}, {"Loading render assets"}});
        Timer timer(true);

        m_renderer.loadAssets(*m_srm);

        SprLog::info({{"[RenderSystem] ", color::GRADIENT20}, {"Finished loading assets in "}, {timer.elapsed()}, {"ms"}});
    }

    ~RenderSystem(){}

    void update(float dt){
        // add created entities
        std::vector<Entity> createdEntities;
        m_ecs->getCreatedEntities<ModelC, TransformC>(createdEntities);
        for (Entity& entity : createdEntities){
            uint32 modelId = m_ecs->get<ModelC>(entity);
            TransformInfo& transform = m_ecs->get<TransformC>(entity);
            m_renderer.insertModel(entity.id, modelId, transform);
        }

        // remove deleted entities
        std::vector<Entity> deletedEntities;
        m_ecs->getDeletedEntities<ModelC, TransformC>(deletedEntities);
        for (Entity& entity : deletedEntities){
            uint32 modelId = m_ecs->get<ModelC>(entity);
            m_renderer.removeModel(entity.id, modelId);
        }

        // update models with changed transform
        std::vector<uint32>& dirtyModels = m_ecs->getDirtyEntityIds<TransformC>();
        for(uint32 entityId : dirtyModels){
            uint32 modelId = m_ecs->get<ModelC>(entityId);
            TransformInfo& transform = m_ecs->get<TransformC>(entityId);
            m_renderer.updateModel(entityId, modelId, transform);
        }

        // lights
        std::vector<Entity> lights;
        m_ecs->getEntities<LightC>(lights);
        for (Entity& entity : lights){
            gfx::Light& light = m_ecs->get<LightC>(entity);
            m_renderer.insertLight(entity.id, light);
        }

        // camera(s)
        std::vector<Entity> cameras;
        m_ecs->getEntities<CameraC>(cameras);
        for (Entity& entity : cameras){
            gfx::Camera& camera = m_ecs->get<CameraC>(entity);
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
        std::vector<Entity> cameras;
        m_ecs->getEntities<CameraC>(cameras);
        gfx::Camera& camera = m_ecs->get<CameraC>(cameras[0]);

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
    ecs.createComponent<TransformC>(transformC, true);
    ecs.createComponent<LightC>(lightC);
    ecs.createComponent<CameraC>(cameraC);
    ecs.createComponent<ModelC>(modelC, true);

    // systems
    InputSystem inputSystem(&ecs, &window);
    RenderSystem renderSystem(&ecs, &window, &rm);
    ecs.createSystem(inputSystem);
    ecs.setRenderSystem(renderSystem);

    // timing
    uint32 frame = 0;

    // scene
    Entity helmet;
    Entity light;
    {
        // models
        helmet = ecs.createEntity(
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

        // ecs.createEntity(
        //     ecs.add<ModelC>(data::sponza),
        //     ecs.add<TransformC>(TransformInfo{
        //         .position = {0.f, 100.f, -5.f}, 
        //         .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
        //         .scale = 0.012f}));

        // ecs.createEntity(
        //     ecs.add<ModelC>(data::bistro),
        //     ecs.add<TransformC>(TransformInfo{
        //         .position = {2.f, 4.f, -2.f}, 
        //         .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
        //         .scale = 1.f}));

        ecs.createEntity(
            ecs.add<ModelC>(data::newsponza_main_gltf_003),
            ecs.add<TransformC>(TransformInfo{
                .position = {2.f, 4.f, -2.f}, 
                .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
                .scale = 1.f}));

        // uint32 dim = 50;
        // for (uint32 x = 0; x < dim; x++){
        //     for (uint32 y = 0; y < dim; y++){
        //         for (uint32 z = 0; z < dim; z++){
        //             ecs.createEntity(
        //                 ecs.add<ModelC>(data::cube),
        //                 ecs.add<TransformC>(TransformInfo{
        //                     .position = {15.f+x, 15.f+y, 15.f+z}, 
        //                     .rotation = angleAxis(0.f, vec3{1.f, 0.f, 0.f}), 
        //                     .scale = 0.1f}));
        //         }
        //     }
        // }

        light = ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .pos = {0.f, 0.f, 0.f},
                .intensity = 8.f,
                .range = 8.f,
                .color = {0.9f, 0.9f, 0.4f}}));
        
        //lights
        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .pos = {13.f, 5.6f, 3.f},
                .intensity = 1.f,
                .range = 16.f,
                .color = {0.9f, 0.55f, 0.89f}}));

        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .pos = {13.f, -5.6f, 3.f},
                .intensity = 1.f,
                .range = 16.f,
                .color = {0.225f, 0.91f, 0.33f}}));

        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .pos = {-13.f, 5.6f, 3.f},
                .intensity = 1.f,
                .range = 16.f,
                .color = {0.27f, 0.88f, 0.94f}}));

        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .pos = {-13.f, -5.6f, 3.f},
                .intensity = 1.f,
                .range = 16.f,
                .color = {0.98f, 0.66f, 0.0f}}));

        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .pos = {0.f, 1.f, 0.f},
                .intensity = 1.f,
                .range = 16.f,
                .color = {1.f, 1.f, 1.0f}}));

        // int32 dim = 12;
        // for (uint32 x = 0; x < dim; x++){
        //     for (uint32 y = 0; y < dim; y++){
        //         for (uint32 z = 0; z < dim; z++){
        //             ecs.createEntity(
        //                 ecs.add<LightC>(gfx::Light{
        //                     .pos = {(-dim/2.0f+x)*2.2f, (-dim/2.0f+y)*2.2f, (-dim/2.0f+z)*2.2f},
        //                     .intensity = 4.f,
        //                     .range = 2.f,
        //                     .color = {(float)x/(float)dim, (float)y/(float)dim, (float)z/(float)dim}}));
        //         }
        //     }
        // }

        ecs.createEntity(
            ecs.add<LightC>(gfx::Light{
                .intensity = 4.f, 
                .dir = glm::normalize(vec3(0.3f, 1.f, -2.f)), 
                .type = gfx::DIRECTIONAL}));

        // camera
        ecs.createEntity(
            ecs.add<CameraC>(gfx::Camera{
                .pos = {0.f, 1.f, 0.f},
                .dir = {0.f, 1.f, 0.f},
                .up  = {0.f, 0.f, 1.f}}));
    }

    // simple game loop
    Timer timer;
    Timer print(true);
    float dt = 8.f;
    uint32 fps = 120.f;
    while (!input.isKeyDown(spr::SPR_ESCAPE)){
        timer.start();

        // update window
        window.update();

        ecs.set<LightC>(light, gfx::Light{
                .pos = {sin(frame/140.f)*4, 0.f, 0.f},
                .intensity = 4.f,
                .range = 8.f,
                .color = {0.9f, 0.9f, 0.8f}});

        ecs.set<TransformC>(helmet, TransformInfo{
                .position = {0.f, 0.f, 0.f}, 
                .rotation = angleAxis((pi<float>()/2)*(frame/240.f), vec3{1.f, 0.f, 0.f}), 
                .scale = 0.2f});

        // update ecs
        ecs.update(dt/1000.f);

        // timing
        timer.stop();
        dt = timer.duration<milliseconds>();
        fps = (1000.f)/dt;

        // print stats
        if (print.elapsed<seconds>() > 1.f){
            print.restart();
            SprLog::info({{"[Main] ", color::GRADIENT20}, {"frame: "}, {frame}});
            SprLog::info({{"       ", color::GRADIENT20}, {"  fps: "}, {fps}});
            SprLog::info({{"       ", color::GRADIENT20}, {"   dt: "}, {dt}, {"ms"}});
        }
        frame++;
    }
}
