#pragma once

namespace spr::gfx {
class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void insertLight();
    void updateCamera();
    void reset();
};
}