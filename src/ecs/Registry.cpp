#include "Registry.h"

namespace spr {
    Registry::Registry() : Registry(512, SPR_REG_SPARSE){}

    Registry::Registry(int size) : Registry(size, SPR_REG_SPARSE) {}

    Registry::Registry(SprRegType registryType) : Registry(512, registryType) {}

    Registry::Registry(int size, SprRegType registryType){
        m_regType = registryType;
        if (registryType == SPR_REG_DENSE){
            m_indicesDense = IndexNode(8);
        } else {
            m_indicesSparse = std::vector<int>(size,-1);
        }
    }

    int Registry::getSize(){
        return m_indicesSparse.size();
    }

    int Registry::getIndex(Entity entity){
        if (m_regType == SPR_REG_DENSE){
            return m_indicesDense.get(entity.id);
        } else {
            return m_indicesSparse.at(entity.id);
        }
    }

    int Registry::getIndex(int id){
        if (m_regType == SPR_REG_DENSE){
            return m_indicesDense.get(id);
        } else {
            return m_indicesSparse.at(id);
        }
    }

    void Registry::addItem(Entity entity, int index){
        if (m_regType == SPR_REG_DENSE){
            m_indicesDense.add(entity.id, index);
        } else {
            while (entity.id > m_indicesSparse.size()){
                m_indicesSparse.resize(m_indicesSparse.size()*2);
            }
            m_indicesSparse.at(entity.id) = index;
        }
    }

    void Registry::addItem(int id, int index){
        if (m_regType == SPR_REG_DENSE){
            m_indicesDense.add(id, index);
        } else {
            while (id > m_indicesSparse.size()){
                m_indicesSparse.resize(m_indicesSparse.size()*2);
            }
            m_indicesSparse.at(id) = index;
        }
    }
}