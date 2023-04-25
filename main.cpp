#include "data/asset_ids.h"
#include "src/interface/Window.h"
#include "src/render/SprRenderer.h"
#include "src/render/scene/Material.h"
#include "src/resource/SprResourceManager.h"
#include "src/debug/SprLog.h"
#include <chrono>
#include <thread>


using namespace spr;

int main() {
    spr::Window window = spr::Window(std::string("Spruce Test"), 1920, 1080);
    window.init();
    InputManager& input = window.getInputManager();
    SprLog::info("[MAIN] Window created");

    gfx::SprRenderer renderer(&window);
    SprLog::info("[MAIN] Renderer created");

    SprResourceManager rm;
    SprLog::info("[MAIN] Resource manager created");

    renderer.loadAssets(rm);
    SprLog::info("[MAIN] Renderer loaded assets");

    while (!input.isKeyDown(spr::SPR_ESCAPE)){
        window.update();
        SprLog::info("[MAIN]     rendering frame");
        // renderer.inserMesh(meshId, materialFlags, ...)
        // renderer.insertLight(...)
        // renderer.updateCamera(...)
        gfx::Transform transform = {{},{}};
        renderer.insertMesh(spr::data::triangle, gfx::MTL_ALL, transform);
        renderer.render();
        SprLog::info("[MAIN]     rendered...");
        std::this_thread::sleep_for(std::chrono::milliseconds(31));
    }

    SprLog::info("[MAIN] Done...");

    return 1;
}