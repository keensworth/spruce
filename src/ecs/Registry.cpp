#include "Registry.h"

namespace spr {
    Registry::Registry(){
        m_indices = std::vector<int>(512);
    }

    Registry::Registry(int size){
        m_indices = std::vector<int>(size);
    }

    int Registry::getIndex(Entity entity){
        return m_indices.at(entity.id);
    }

    int Registry::getIndex(int id){
        return m_indices.at(id);
    }

    void Registry::addItem(Entity entity, int index){
        while (entity.id > m_indices.size()){
            m_indices.resize(m_indices.size()*2);
        }
        m_indices.at(entity.id) = index;
    }

    void Registry::addItem(int id, int index){
        while (id > m_indices.size()){
            m_indices.resize(m_indices.size()*2);
        }
        m_indices.at(id) = index;
    }
}