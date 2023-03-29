#pragma once
#include "resource/VulkanResourceManager.h"

namespace spr::gfx {
class StagingBuffers {
public:
    StagingBuffers() {}
    StagingBuffers(VulkanResourceManager* rm) {
        m_rm = rm;

        // init staging buffers
        m_stage64MB = rm->create<Buffer>(BufferDesc{
            .byteSize = m_size64MB, 
            .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
            .memType = (HOST)
        });

        for (uint32 i = 0; i < m_maxCount16MB; i++) {
            m_stages16MB[i] = rm->create<Buffer>(BufferDesc{
                .byteSize = m_size16MB, 
                .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
                .memType = (HOST)
            });
        }

        for (uint32 i = 0; i < m_maxCount4MB; i++) {
            m_stages4MB[i] = rm->create<Buffer>(BufferDesc{
                .byteSize = m_size4MB, 
                .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
                .memType = (HOST)
            });
        }

        for (uint32 i = 0; i < m_maxCount1MB; i++) {
            m_stages1MB[i] = rm->create<Buffer>(BufferDesc{
                .byteSize = m_size1MB, 
                .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
                .memType = (HOST)
            });
        }

        m_count64MB = 0;
        m_count16MB = 0;
        m_count4MB  = 0;
        m_count1MB  = 0;
    }

    ~StagingBuffers() {     
        m_rm->remove(m_stage64MB);
        
        for (uint32 i = 0; i < m_maxCount16MB; i++) {
            m_rm->remove(m_stages16MB[i]);
        }

        for (uint32 i = 0; i < m_maxCount4MB; i++) {
            m_rm->remove(m_stages4MB[i]);
        }

        for (uint32 i = 0; i < m_maxCount1MB; i++) {
            m_rm->remove(m_stages1MB[i]);
        }
    }

    Handle<Buffer> getStagingBuffer(uint32 sizeBytes) {
        // try 1MB staging buffer
        if (sizeBytes <= m_size1MB && m_count1MB < m_maxCount1MB){
            Handle<Buffer> buffer = m_stages1MB[m_count1MB];
            m_count1MB++;
            return buffer;
        }

        // try 4MB staging bufer
        if (sizeBytes <= m_size4MB && m_count4MB < m_maxCount4MB){
            Handle<Buffer> buffer = m_stages4MB[m_count4MB];
            m_count4MB++;
            return buffer;
        }

        // try 16MB staging buffer
        if (sizeBytes <= m_size16MB && m_count16MB < m_maxCount16MB){
            Handle<Buffer> buffer = m_stages16MB[m_count16MB];
            m_count16MB++;
            return buffer;
        }

        // try 64MB staging buffer
        if (sizeBytes <= m_size64MB && m_count64MB < m_maxCount64MB){
            Handle<Buffer> buffer = m_stage64MB;
            m_count64MB++;
            return buffer;
        }

        // none available
        // create new staging buffer
        Handle<Buffer> overflowStage = m_rm->create<Buffer>(BufferDesc{
            .byteSize = sizeBytes, 
            .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
            .memType = (HOST)
        });
        m_overflowStages.push_back(overflowStage);
        return overflowStage;
    }

    void reset() {
        m_count64MB = 0;
        m_count16MB = 0;
        m_count4MB  = 0;
        m_count1MB  = 0;
        
        if (m_overflowStages.size() == 0)
            return;

        // destroy and deallocate extra staging buffers
        for (Handle<Buffer> buffer : m_overflowStages){
            m_rm->remove(buffer);
        }

        m_overflowStages = std::vector<Handle<Buffer>>();
    }

private:
    static const uint32 m_maxCount64MB = 1;
    static const uint32 m_maxCount16MB = 4;
    static const uint32 m_maxCount4MB  = 16;
    static const uint32 m_maxCount1MB  = 64;

    static const uint32 m_size64MB = 1u << 25; // 2^26 bytes (64MB)
    static const uint32 m_size16MB = 1u << 23; // 2^24 bytes (16MB)
    static const uint32 m_size4MB = 1u << 21;  // 2^22 bytes (4MB)
    static const uint32 m_size1MB = 1u << 19;  // 2^20 bytes (1MB)

private:
    // 256 MB (total) pre-allocated staging buffers
    Handle<Buffer> m_stage64MB;                  // 1x 64MB 
    Handle<Buffer> m_stages16MB[m_maxCount16MB]; // 4x 16MB
    Handle<Buffer> m_stages4MB[m_maxCount4MB];   // 16x 4MB
    Handle<Buffer> m_stages1MB[m_maxCount1MB];   // 64x 1MB
    std::vector<Handle<Buffer>> m_overflowStages;

    uint32 m_count64MB = 0;
    uint32 m_count16MB = 0;
    uint32 m_count4MB  = 0;
    uint32 m_count1MB  = 0;

    VulkanResourceManager* m_rm;
};
}