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
    int getIndex(uint32 id);
    void addItem(Entity entity, int32 index);
    void addItem(uint32 id, int32 index);

    int getSize();
private:
    int m_regType;
    IndexNode m_indicesDense;
    std::vector<int32> m_indicesSparse;
};

}