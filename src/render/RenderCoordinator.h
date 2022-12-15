#pragma once

#include "util/BatchManager.h"
#include "spruce_core.h"


namespace spr::gfx{

typedef struct {
    glm::vec3 camPosition;
    float fovX;
    glm::vec3 camDirection;
    float fovY;
} Camera;

class RenderCoordinator{
public:
    RenderCoordinator(){
        
    }
    ~RenderCoordinator(){}

    void insertMesh();
    void insertLight();
    void updateCamera();
    void render(BatchManager* batchManager);

    void destroy();

private:

};
}