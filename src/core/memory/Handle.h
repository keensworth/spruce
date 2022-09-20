#pragma once

#include "../Core.h"


namespace spr{
template <typename T>
class Pool; 

template <typename T>
class Handle {
public:
    Handle() : m_index(0), m_generation(0) {}
    ~Handle() {}
    bool isValid() { return m_generation != 0; }

private:
    Handle(uint32 index, uint32 generation) : m_index(index), m_generation(generation) {}

    uint32 m_index;
    uint32 m_generation;

    friend class Pool<T>;
};

}