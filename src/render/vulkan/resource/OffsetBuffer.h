#pragma once

#include <utility>
#include <vector>
#include "spruce_core.h"
#include <string>
#include <cstring>
#include "debug/SprLog.h"
#include "vulkan/resource/VulkanResourceManager.h"

namespace spr::gfx {

template <typename T>
struct AllocationInfo {
    // typed data ptr
    T* data;

    // size in elements
    uint32 size;
};

template <typename T>
struct OffsetBufferAllocation {
    // typed ptr to start of alloc
    T* ptr;

    // offset in elements
    uint32 offset;

    // size in elements
    uint32 size;

    // offset in bytes
    uint32 byteOffset;

    // size in bytes
    uint32 byteSize;
};


class OffsetBuffer {
public:
    OffsetBuffer() {
        m_capacity = 0;
        m_size = 0;
        m_rm = nullptr;
    }

    OffsetBuffer(VulkanResourceManager* rm, uint32 capacity) {
        m_rm = rm;
        m_size = 0;
        m_capacity = m_rm->alignedSize(capacity);

        m_handle = rm->create<Buffer>({
            .byteSize = m_rm->alignedSize(capacity), 
            .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
            .memType = (HOST)
        });

        m_buffer = m_rm->get(m_handle);
        m_buffer->byteSize = 0;
        m_dataPtr = (uint8*)m_buffer->allocInfo.pMappedData;
        m_offsetPtr = m_dataPtr;
    }

    ~OffsetBuffer() {
    }

    // OffsetBuffer(const OffsetBuffer&) = delete;
    // OffsetBuffer& operator=(const OffsetBuffer&) = delete;

    // OffsetBuffer(OffsetBuffer&& other) noexcept
    //     : m_handle(std::move(other.m_handle)), m_buffer(other.m_buffer),
    //     m_capacity(other.m_capacity), m_size(other.m_size),
    //     m_destroyed(other.m_destroyed), m_rm(other.m_rm),
    //     m_dataPtr(other.m_dataPtr), m_offsetPtr(other.m_offsetPtr) {
    //     other.m_buffer = nullptr;
    //     other.m_dataPtr = nullptr;
    //     other.m_offsetPtr = nullptr;
    // }

    // OffsetBuffer& operator=(OffsetBuffer&& other) noexcept {
    //     if (this != &other) {
    //         destroy();
    //         m_handle = std::move(other.m_handle);
    //         m_buffer = other.m_buffer;
    //         m_capacity = other.m_capacity;
    //         m_size = other.m_size;
    //         m_destroyed = other.m_destroyed;
    //         m_rm = other.m_rm;
    //         m_dataPtr = other.m_dataPtr;
    //         m_offsetPtr = other.m_offsetPtr;

    //         other.m_buffer = nullptr;
    //         other.m_dataPtr = nullptr;
    //         other.m_offsetPtr = nullptr;
    //     }
    //     return *this;
    // }

public:
    // Input: 
    //      uint32 size - number of T to be inserted:
    //                    sizeBytes = size*sizeof(T)
    // Output: 
    //      OffsetBufferAllocation<T> - allocation info
    template <typename T = const unsigned char>
    inline OffsetBufferAllocation<T> allocate(uint32 size){
        // element-wise offset
        uint32 offsetIndex = m_size / sizeof(T);
        uint32 offsetBytes = m_size;

        // ptr to start of new allocation
        uint8* allocPtr = m_offsetPtr;
        
        // check that data can fit
        uint32 sizeBytes = size * sizeof(T);
        if (m_size + sizeBytes > m_capacity){
            sizeBytes = m_capacity-m_size;
            size = sizeBytes / sizeof(T);
            SprLog::warn("[OffsetBuffer] [allocate] " + std::string("size > capacity, writing partial data"));
        }
        
        // adjust sizes and offset
        m_size += sizeBytes;
        m_offsetPtr += sizeBytes;
        m_buffer->byteSize += sizeBytes;

        return {(T*)allocPtr, offsetIndex, size, offsetBytes, sizeBytes};
    }

    // Input: 
    //      T* data     - data to be stored in buffer
    //      uint32 size - number of T to be inserted:
    //                    sizeBytes = size*sizeof(T)
    // Output: 
    //      OffsetBufferAllocation<T> - allocation info
    template <typename T = const unsigned char>
    inline OffsetBufferAllocation<T> allocateAndInsert(const T* data, uint32 size){
        // offsets (typed/index, bytes)
        uint32 offsetIndex = m_size / sizeof(T);
        uint32 offsetBytes = m_size;

        // ptr to start of new allocation
        uint8* allocPtr = m_offsetPtr;
        
        // check that data can fit
        uint32 sizeBytes = size * sizeof(T);
        if (m_size + sizeBytes > m_capacity){
            sizeBytes = m_capacity-m_size;
            size = sizeBytes / sizeof(T);
            SprLog::warn("[OffsetBuffer] [allocateAndInsert] " + std::string("size > capacity, writing partial data"));
        }
        
        // copy data into buffer
        std::memcpy(allocPtr, data, sizeBytes);
        
        // adjust sizes and offset
        m_size += sizeBytes;
        m_offsetPtr += sizeBytes;
        m_buffer->byteSize += sizeBytes;

        return {(T*)allocPtr, offsetIndex, size, offsetBytes, sizeBytes};
    }

    // Input: 
    //      AllocationInfo<T> allocInfo     - data to be stored in buffer
    // Output: 
    //      OffsetBufferAllocation<T>       - allocation info
    template <typename T = const unsigned char>
    inline OffsetBufferAllocation<T> allocateAndInsert(AllocationInfo<T> allocInfo){
        return allocateAndInsert(allocInfo.data, allocInfo.size);
    }

    // Input: 
    //      AllocationInfo<T> allocInfo     - data to be stored in buffer
    // Output: 
    //      OffsetBufferAllocation<T>       - allocation info
    template <typename T = const unsigned char>
    inline OffsetBufferAllocation<T> allocateAndInsert(AllocationInfo<const unsigned char> allocInfo){
        return allocateAndInsert((T*)allocInfo.data, allocInfo.size / sizeof(T));
    }

    // Input: 
    //      T& data     - data to be stored in buffer
    // Output: 
    //      OffsetBufferAllocation<T> - allocation info
    template <typename T = const unsigned char>
    inline OffsetBufferAllocation<T> allocateAndInsert(const T& data){
        // offsets (typed/index, bytes)
        uint32 offsetIndex = m_size / sizeof(T);
        uint32 offsetBytes = m_size;
        uint32 size = 1;

        // ptr to start of new allocation
        uint8* allocPtr = m_offsetPtr;
        
        // check that data can fit
        uint32 sizeBytes = size * sizeof(T);
        if (m_size + sizeBytes > m_capacity){
            sizeBytes = m_capacity-m_size;
            size = sizeBytes / sizeof(T);
            SprLog::warn("[OffsetBuffer] [allocateAndInsert] " + std::string("size > capacity, writing partial data"));
        }
        
        // copy data into buffer
        std::memcpy(allocPtr, &data, sizeBytes);
        
        // adjust sizes and offset
        m_size += sizeBytes;
        m_offsetPtr += sizeBytes;
        m_buffer->byteSize += sizeBytes;

        return {(T*)allocPtr, offsetIndex, size, offsetBytes, sizeBytes};
    }

    // base pointer
    uint8* data(){
        return (uint8*)(m_dataPtr);
    }

    // offset pointer (next write location)
    uint8* offset(){
        return m_offsetPtr;
    }

    // size in bytes
    uint32 size(){
        return m_size;
    }

    // capacity in bytes
    uint32 capacity(){
        return m_capacity;
    }

    Handle<Buffer> handle(){
        return m_handle;
    }

    void clear(){
        m_offsetPtr = m_dataPtr;
        m_size = 0;
    }

    void destroy(){
        if (m_destroyed)
            return;
        m_rm->remove(m_handle);
        m_dataPtr = nullptr;
        m_offsetPtr = m_dataPtr;
        m_size = 0;

        m_destroyed = true;
    }


private:
    Handle<gfx::Buffer> m_handle;
    gfx::Buffer* m_buffer;

    // capacity, size in bytes
    uint32 m_capacity;
    uint32 m_size = 0;
    bool m_destroyed = false;

    // non-owning
    VulkanResourceManager* m_rm;
    uint8* m_dataPtr;
    uint8* m_offsetPtr;
};

}