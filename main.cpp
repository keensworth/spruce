#include "KeyboardConfig.h"
#include "data/asset_ids.h"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "src/interface/Window.h"
#include "src/render/SprRenderer.h"
#include "src/render/scene/Material.h"
#include "src/resource/ResourceTypes.h"
#include "src/resource/SprResourceManager.h"
#include "src/debug/SprLog.h"
#include <chrono>
#include <thread>
#include "glm/gtx/string_cast.hpp"
#include <chrono>

using namespace spr;

void handleInput(InputManager& input, glm::vec3& pos, glm::vec3& dir, glm::vec3& up, glm::vec3& right){
    if (input.isKeyDown(spr::SPR_w)){
        pos += 0.12f*dir;
    }
    if (input.isKeyDown(spr::SPR_s)){
        pos += -0.12f*dir;
    }
    if (input.isKeyDown(spr::SPR_a)){
        pos += -0.12f*right;
    }
    if (input.isKeyDown(spr::SPR_d)){
        pos += 0.12f*right;
    }
    if (input.isKeyDown(spr::SPR_SPACE)){
        pos += vec3{0.f, 0.f, 0.04f};
    }
    if (input.isKeyDown(spr::SPR_LSHIFT)){
        pos += vec3{0.f, 0.f, -0.12f};
    }

    float yaw = 0;
    if (input.isKeyDown(spr::SPR_LEFT)){
        yaw = 0.02;
    } else if (input.isKeyDown(spr::SPR_RIGHT)){
        yaw = -0.02;
    }

    float pitch = 0;
    if (input.isKeyDown(spr::SPR_UP)){
        pitch = 0.02;
    } else if (input.isKeyDown(spr::SPR_DOWN)){
        pitch = -0.02;
    }

    // update camera
    dir = rotate(angleAxis(yaw, vec3{0.f, 0.f, 1.f}), dir);
    up = rotate(angleAxis(yaw, vec3{0.f, 0.f, 1.f}), up);
    right = rotate(angleAxis(yaw, vec3{0.f, 0.f, 1.f}), right);
    dir = rotate(angleAxis(pitch, right), dir);
    up = rotate(angleAxis(pitch, right), up);
}


int main() {
    // window and input
    spr::Window window = spr::Window(std::string("Spruce Test"), 1920, 1080);
    window.init();
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

    // simple game loop
    while (!input.isKeyDown(spr::SPR_ESCAPE)){
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
        renderer.insertModel(data::sponza, {
            .position = {0.f, 0.f, -5.f}, 
            .rotation = angleAxis(pi<float>()/2, vec3{1.f, 0.f, 0.f}), 
            .scale = 0.02f
        });
        
        // lights
        renderer.insertLight({.pos = {3.f, 0.f, -1.f}, .intensity = 1.f, .range = 32.f});
        renderer.insertLight({.pos = {20.f * glm::sin(frame/500.f), 0.f, -1.f}, .intensity = 1.f, .range = 32.f});
        renderer.insertLight({.pos = {-3.f, 0.f, -1.f}, .intensity = 1.f, .range = 32.f});

        // handle input + update camera
        handleInput(input, pos, dir, up, right);
        renderer.updateCamera({.pos = pos, .dir = dir, .up = up});

        // render 
        renderer.render();

        // timing + window update
        stop = std::chrono::high_resolution_clock::now();
        auto printDur = (stop - printTime);
        if (printDur.count() > 1000000000){
            printTime = std::chrono::high_resolution_clock::now();
            auto dur = (stop - start);
            SprLog::info("[MAIN] frame: ", frame);
            SprLog::info("[MAIN] fps: " + std::to_string((float)(1000000000)/dur.count()) );
        }
        window.update();
        frame++;

    }

    return 1;
}