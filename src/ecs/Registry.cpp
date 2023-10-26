#include "Registry.h"

namespace spr {
    Registry::Registry() : Registry(512, SPR_REG_SPARSE){}

    Registry::Registry(uint32 size) : Registry(size, SPR_REG_SPARSE) {}

    Registry::Registry(SprRegType registryType) : Registry(512, registryType) {}

    Registry::Registry(uint32 size, SprRegType registryType){
        m_regType = registryType;
        if (registryType == SPR_REG_DENSE){
            m_indicesDense = IndexNode(8);
        } else {
            m_indicesSparse = std::vector<int32>(size, -1);
        }
    }

    uint32 Registry::size(){
        return m_indicesSparse.size();
    }

    int32 Registry::getIndex(Entity& entity){
        if (m_regType == SPR_REG_DENSE){
            return m_indicesDense.get(entity.id);
        } else {
            return m_indicesSparse.at(entity.id);
        }
    }

    int32 Registry::getIndex(uint32 id){
        if (m_regType == SPR_REG_DENSE){
            return m_indicesDense.get(id);
        } else {
            return m_indicesSparse.at(id);
        }
    }

    void Registry::addItem(Entity& entity, int32 index){ 
        if (m_regType == SPR_REG_DENSE){
            m_indicesDense.add(entity.id, index);
        } else {
            while (entity.id >= m_indicesSparse.size()){
                m_indicesSparse.resize(m_indicesSparse.size()*2);
            }
            m_indicesSparse.at(entity.id) = index;
        }
    }

    void Registry::addItem(uint32 id, int32 index){
        if (m_regType == SPR_REG_DENSE){
            m_indicesDense.add(id, index);
        } else {
            while (id >= m_indicesSparse.size()){
                m_indicesSparse.resize(m_indicesSparse.size()*2);
            }
            m_indicesSparse.at(id) = index;
        }
    }
}