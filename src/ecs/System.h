#pragma once

#include <vector>
#include "SprECS.h"

namespace spr {

class System {
public:
    System(SprECS& ecs) : ecs(ecs) {}
    ~System() {}

    virtual void update(float dt);
private:
    SprECS& ecs;
};

}