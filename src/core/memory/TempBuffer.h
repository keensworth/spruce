#pragma once

#include <utility>
#include <vector>
#include "../spruce_core.h"
#include <stdio.h>
#include <string>
#include <cstring>
#include "../../debug/SprLog.h"

namespace spr {

template <typename T>
class TempBuffer {
public:
    TempBuffer() {
        m_capacity = 4096;
        m_size = 0;
        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
        m_offsetPointer = m_data;
    }

    TempBuffer(uint32 capacity) {
        m_capacity = capacity;
        m_size = 0;
        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
        m_offsetPointer = m_data;
    }

    ~TempBuffer() {
        free(m_data);
    }

public:
    // Input: 
    //      T* data     - data to be stored in buffer
    //      uint32 size - number of T to be inserted
    // Output: 
    //      uint32 ofst - offset (T) of data in buffer
    uint32 insert(T* data, uint32 size){
        // current offset
        uint32 offset = m_size;
        
        // check that data can fit
        if (m_size + size >= m_capacity){
            size = m_capacity-m_size;
            SprLog::warn("TempBuffer: " + std::string("size >= capacity, writing partial data"));
        }

        // emplace data into buffer
        m_offsetPointer = static_cast<T*>(memcpy(m_offsetPointer, data, size*sizeof(T)));

        // increase size;
        m_size += size;

        return offset;
    }

    uint32 insert(T data){
        insert(*data, 1);
    }

    T* getData(){
        return m_data;
    }

    uint32 getSize(){
        return m_size;
    }

    uint32 getCapacity(){
        return m_capacity;
    }

    void reset(){
        free(m_data);
        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
        m_offsetPointer = m_data;
        m_size = 0;
    }


private:
    T* m_data;
    T* m_offsetPointer;
    uint32 m_capacity;
    uint32 m_size = 0;
};

}