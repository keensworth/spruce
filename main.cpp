#include "KeyboardConfig.h"
#include "data/asset_ids.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "src/interface/Window.h"
#include "src/render/SprRenderer.h"
#include "src/render/scene/Material.h"
#include "src/render/scene/SceneData.h"
#include "src/resource/ResourceTypes.h"
#include "src/resource/SprResourceManager.h"
#include "src/debug/SprLog.h"
#include <chrono>
#include <thread>
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/vector_angle.hpp>
#include <chrono>

using namespace spr;

void handleInput(InputManager& input, vec3& pos, vec3& dir, vec3& up, vec3& right, spr::Window& window, float dt){
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
    spr::Window window = spr::Window(std::string("Spruce Test"), 1920, 1080);
    window.init();
    window.setRelativeMouse(true);
    InputManager& input = window.getInputManager();

    // renderer and assets
    gfx::SprRenderer renderer(&window);
    SprResourceManager rm;
    renderer.loadAssets(rm);

    // initial camera config
    vec3 pos = {0.f, 1.f, 0.f};
    vec3 dir = {0.f, 1.f, 0.f};
    vec3 up = {0.f, 0.f, 1.f};
    vec3 right = {1.f, 0.f, 0.f};

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
        
        //insert models using their ids directly
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
            .rotation = angleAxis(0.f, vec3{1.f, 0.f, 0.f}), 
            .scale = 3.f
        });
        renderer.insertModel(data::boomboxwithaxes, {
            .position = {-2.5f, 0.f, -1.f}, 
            .rotation = angleAxis(pi<float>()/2.f, vec3{1.f, 0.f, 0.f}), 
            .scale = 10.f
        });
        renderer.insertModel(data::sponza, {
            .position = {0.f, 0.f, -5.f}, 
            .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
            .scale = 0.012f
        });
        
        // lights
        renderer.insertLight({.pos = {3.f, 0.f, -1.f}, .intensity = 1.f, .range = 16.f}); // point
        renderer.insertLight({.pos = {-3.f, 0.f, -1.f}, .intensity = 1.f, .range = 16.f});// point
        renderer.insertLight({.intensity = 1.2f, .dir = {0.2f, 0.f, -1.f}, .type = gfx::DIRECTIONAL});

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
        }
        frame++;
    }

    return 1;
}