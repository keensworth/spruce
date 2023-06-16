#pragma once

#include "../spruce_core.h"


namespace spr{
template <typename T>
class Pool; 

template <typename T>
class Handle {
public:
    Handle() : m_index(0), m_generation(0) {}
    ~Handle() {}
    bool isValid() const { return m_generation != 0; }
    bool operator==(const Handle& rhs) const{
        return (this->m_index == rhs.m_index) && (this->m_generation == rhs.m_generation);
    }
    bool operator!=(const Handle& rhs) const{
        return (this->m_index != rhs.m_index) || (this->m_generation != rhs.m_generation);
    }

// private:
    Handle(uint32 index, uint32 generation) : m_index(index), m_generation(generation) {}

    uint32 m_index = 0;
    uint32 m_generation = 0;

private:

    friend class Pool<T>;
};

}