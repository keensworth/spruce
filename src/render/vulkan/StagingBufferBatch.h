#pragma once
#include "resource/VulkanResourceManager.h"

namespace spr::gfx {
class StagingBuffers {
public:
    StagingBuffers();
    StagingBuffers(VulkanResourceManager* rm);
    ~StagingBuffers();

    Handle<Buffer> getStagingBuffer(uint32 sizeBytes);
    void reset();

private:
    // quantity of buffers at each scale
    static const uint32 m_maxCount64MB = 1;
    static const uint32 m_maxCount16MB = 4;
    static const uint32 m_maxCount4MB  = 16;
    static const uint32 m_maxCount1MB  = 64;

    // size of buffers at each scale
    static const uint32 m_size64MB = (1u << 25); // 2^26 bytes (64MB)
    static const uint32 m_size16MB = (1u << 23); // 2^24 bytes (16MB)
    static const uint32 m_size4MB  = (1u << 21); // 2^22 bytes (4MB)
    static const uint32 m_size1MB  = (1u << 19); // 2^20 bytes (1MB)

private: // owning
    // 256 MB (total) pre-allocated staging buffers
    Handle<Buffer> m_stage64MB;                  //  1 x 64MB 
    Handle<Buffer> m_stages16MB[m_maxCount16MB]; //  4 x 16MB
    Handle<Buffer> m_stages4MB[m_maxCount4MB];   // 16 x  4MB
    Handle<Buffer> m_stages1MB[m_maxCount1MB];   // 64 x  1MB
    std::vector<Handle<Buffer>> m_overflowStages;

private: // non-owning
    VulkanResourceManager* m_rm;

    uint32 m_count64MB = 0;
    uint32 m_count16MB = 0;
    uint32 m_count4MB  = 0;
    uint32 m_count1MB  = 0;

    uint32 m_totalOverflowSizeBytes = 0;
};
}