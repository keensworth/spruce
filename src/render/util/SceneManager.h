#pragma once

#include "spruce_core.h"
#include "BatchManager.h"
#include <vector>

namespace spr::gfx {

typedef struct Camera {
    glm::vec3 camPosition;
    float fovX;
    glm::vec3 camDirection;
    float fovY;
} Camera;

typedef struct Light {

} Light;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    void insertLight();
    void insertMesh(uint32 meshId, uint32 materialFlags, glm::mat4 model, glm::mat4 modelInvTranspose);
    void updateCamera();
    void reset(){
        m_batchManager.reset();
    }

private:
    Camera m_camera;
    std::vector<Light> m_lights;
    BatchManager m_batchManager;
};
}