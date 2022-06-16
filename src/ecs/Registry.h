#pragma once

#include <vector>
#include "Entity.h"
#include "../core/util/node/IndexNode.h"

namespace spr {

typedef enum {
    SPR_REG_SPARSE,
    SPR_REG_DENSE
} SprRegType;

class Registry {
public:
    Registry();
    Registry(int size);
    Registry(SprRegType registryType);
    Registry(int size, SprRegType registryType);
    ~Registry() {}

    int getIndex(Entity entity);
    int getIndex(int id);
    void addItem(Entity entity, int index);
    void addItem(int id, int index);
private:
    int m_regType;
    IndexNode m_indicesDense;
    std::vector<int> m_indicesSparse;
};

}