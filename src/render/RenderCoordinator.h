#pragma once

#include "util/SceneManager.h"



namespace spr::gfx{

class RenderCoordinator{
public:
    RenderCoordinator();
    ~RenderCoordinator();

    void render(SceneManager& sceneManager);
    void destroy();

private:

};
}