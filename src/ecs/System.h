#pragma once

#include <vector>

namespace spr {
class System {
public:
    System() {}
    ~System() {}

    virtual void update(float dt) = 0;
private:
};

}