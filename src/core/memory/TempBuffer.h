#pragma once

#include <utility>
#include <vector>
#include "../spruce_core.h"
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

    TempBuffer(T* data, uint32 size) {
        m_capacity = size;
        m_size = size;
        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
        m_offsetPointer = static_cast<T*>(memcpy(m_offsetPointer, data, size*sizeof(T)));
    }

    ~TempBuffer() {
        free(m_data);
    }

    // copy constructor
    TempBuffer(const TempBuffer& other) {
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
        m_offsetPointer = m_data + (other.m_offsetPointer - other.m_data);
        std::memcpy(m_data, other.m_data, m_size * sizeof(T));
    }

    // copy assignment operator
    TempBuffer& operator=(const TempBuffer& other) {
        if (this != &other) {
            free(m_data);
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
            m_offsetPointer = m_data + (other.m_offsetPointer - other.m_data);
            std::memcpy(m_data, other.m_data, m_size * sizeof(T));
        }
        return *this;
    }

public:
    // Input: 
    //      T* data     - data to be stored in buffer
    //      uint32 size - number of T to be inserted
    // Output: 
    //      uint32 ofst - offset (T) of data in buffer
    inline uint32 insert(const T* data, uint32 size){
        // current offset
        uint32 offset = m_size;
        
        // check that data can fit
        if (m_size + size > m_capacity){
            size = m_capacity-m_size;
            SprLog::warn("[TempBuffer] [insert] " + std::string("size >= capacity, writing partial data"));
        }

        // emplace data into buffer
        std::memcpy(m_offsetPointer, data, size*sizeof(T));
        m_offsetPointer += size;

        // increase size;
        m_size += size;

        return offset;
    }

    inline uint32 insert(const T& data){
        // current offset
        uint32 offset = m_size;
        
        // check that data can fit
        uint32 size = 1;
        if (m_size + size > m_capacity){
            size = m_capacity-m_size;
            SprLog::warn("[TempBuffer] [insert] " + std::string("size >= capacity, writing partial data"));
        }

        // emplace data into buffer
        std::memcpy(m_offsetPointer, &data, size*sizeof(T));
        m_offsetPointer += size;
        // increase size;
        m_size += size;

        return offset;
    }

    // returns typed pointer
    T* getData(){
        return m_data;
    }

    uint8* getBytes(){
        return (uint8*)(m_data);
    }

    // returns typed size
    uint32 getSize(){
        return m_size;
    }

    uint32 getSizeBytes(){
        return m_size * sizeof(T);
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