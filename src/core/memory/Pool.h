#pragma once

#include <utility>
#include <vector>
#include "Handle.h"
#include "../spruce_core.h"
#include <stdio.h>
#include <string>
#include "../../debug/SprLog.h"

namespace spr {

template <typename T>
class Pool {
public:
    Pool() {
        m_capacity = 64;
        m_size = 0;
        m_freeListIndex = 0;
        m_freeList = std::vector<uint32>();
        for (int i = 0; i < 64; i++){
            m_freeList.push_back(i);
        }
        m_generations = std::vector<uint32>(64,1);

        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
    }

    Pool(uint32 capacity) {
        m_capacity = capacity;
        m_size = 0;
        m_freeListIndex = 0;
        m_freeList = std::vector<uint32>();
        for (int i = 0; i < m_capacity; i++){
            m_freeList.push_back(i);
        }
        m_generations = std::vector<uint32>(m_capacity,1);

        m_data = static_cast<T*>(malloc(m_capacity * sizeof(T)));
    }

    ~Pool() {
        for (int i = 0 ; i < m_size; i++){
            if (m_data + i)
                (m_data + i)->~T();
        }
        free(m_data);
    }

private:
    void resize(){
        // allocate new memory
        uint32 newCapacity = m_capacity * 2;
        T* newData = static_cast<T*>(malloc(newCapacity * sizeof(T)));

        // copy current mem into 2nd half of new
        memcpy(newData, m_data, m_capacity * sizeof(T));

        // resize freelist and generations
        for (int i = m_capacity; i < newCapacity; i++){
            m_freeList.push_back(i);
            m_generations.push_back(1);
        }

        // delete old mem
        for (int i = 0 ; i < m_size; i++){
            (m_data + i)->~T();
        }
        free(m_data);

        // set pointer to new mem
        m_data = newData;
    }

    template <typename... Args>
    Handle<T> emplace(Args&&... args){
        // out of room, resize
        if (m_freeListIndex >= m_capacity){
            resize();
        }

        Handle<T> handle;
        handle.m_index = m_freeList[m_freeListIndex];
        handle.m_generation = m_generations[handle.m_index];
        m_freeListIndex++;

        // place element in array
        new (m_data + handle.m_index) T(std::forward<Args>(args)...);
        m_size++;

        return handle;
    }

public:
    Handle<T> insert(const T& data){
        return emplace(data);
    }

    Handle<T> remove(Handle<T> handle){
        // verify that data exists at handle, otherwise return
        if (!(m_data + handle.m_index))
            return Handle<T>();
        
        // delete data at handle
        (m_data + handle.m_index)->~T();

        // increase generation at that index
        m_generations[handle.m_index] += 1;

        // increase freelistindex
        m_freeListIndex++;

        // add index to freelist at freelistindex
        m_freeList[m_freeListIndex] = handle.m_index;
        m_size--;

        return Handle<T>();
    }

    T* get(Handle<T> handle){
        if (!isValidHandle(handle))
            return nullptr;
        return (m_data + handle.m_index);
    }

    bool isValidHandle(Handle<T> handle){
        return (m_generations.at(handle.m_index) == handle.m_generation);
    }


private:
    T* m_data;
    uint32 m_capacity;
    uint32 m_size = 0;

    uint32 m_freeListIndex;
    std::vector<uint32> m_freeList;

    std::vector<uint32> m_generations;
};

}