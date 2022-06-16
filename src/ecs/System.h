#pragma once

#include <vector>
#include "Entity.h"
#include "SprECS.h"
#include "../core/util/node/IndexNode.h"

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