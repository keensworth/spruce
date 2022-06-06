#pragma once

#include <Entity.h>
#include <vector>

namespace spr {

class Registry {
public:
    Registry();
    Registry(int size);
    ~Registry() {}

    int getIndex(Entity entity);
    int getIndex(int id);
    void addItem(Entity entity, int index);
    void addItem(int id, int index);
private:
    std::vector<int> m_indices;
};

}