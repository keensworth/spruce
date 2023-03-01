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

        for (uint32 i = 0; i < 4; i++) {
            m_stages16MB[i] = rm->create<Buffer>(BufferDesc{
                .byteSize = m_size16MB, 
                .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
                .memType = (HOST)
            });
        }

        for (uint32 i = 0; i < 16; i++) {
            m_stages4MB[i] = rm->create<Buffer>(BufferDesc{
                .byteSize = m_size4MB, 
                .usage = Flags::BufferUsage::BU_TRANSFER_SRC,
                .memType = (HOST)
            });
        }

        for (uint32 i = 0; i < 64; i++) {
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

        for (uint32 i = 0; i < 4; i++) {
            m_rm->remove(m_stages16MB[i]);

        }

        for (uint32 i = 0; i < 16; i++) {
            m_rm->remove(m_stages4MB[i]);

        }

        for (uint32 i = 0; i < 64; i++) {
            m_rm->remove(m_stages1MB[i]);
        }
    }

    Handle<Buffer> getStagingBuffer(uint32 sizeBytes) {
        // try 1MB staging buffer
        if (sizeBytes <= m_size1MB && m_count1MB < 64){
            Handle<Buffer> buffer = m_stages1MB[m_count1MB];
            m_count1MB++;
            return buffer;
        }

        // try 4MB staging bufer
        if (sizeBytes <= m_size4MB && m_count4MB < 16){
            Handle<Buffer> buffer = m_stages4MB[m_count4MB];
            m_count4MB++;
            return buffer;
        }

        // try 16MB staging buffer
        if (sizeBytes <= m_size16MB && m_count16MB < 4){
            Handle<Buffer> buffer = m_stages16MB[m_count16MB];
            m_count16MB++;
            return buffer;
        }

        // try 64MB staging buffer
        if (sizeBytes <= m_size64MB && m_count64MB < 1){
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
    // 256 MB (total) pre-allocated staging buffers
    Handle<Buffer> m_stage64MB;     // 1x 64MB 
    Handle<Buffer> m_stages16MB[4]; // 4x 16MB
    Handle<Buffer> m_stages4MB[16]; // 16x 4MB
    Handle<Buffer> m_stages1MB[64]; // 64x 1MB
    std::vector<Handle<Buffer>> m_overflowStages;

    uint32 m_count64MB = 0;
    uint32 m_count16MB = 0;
    uint32 m_count4MB  = 0;
    uint32 m_count1MB  = 0;

    uint32 m_size64MB = 1u << 25; // 2^26 bytes (64MB)
    uint32 m_size16MB = 1u << 23; // 2^24 bytes (16MB)
    uint32 m_size4MB = 1u << 21;  // 2^22 bytes (4MB)
    uint32 m_size1MB = 1u << 19;  // 2^20 bytes (1MB)

    VulkanResourceManager* m_rm;
};
}