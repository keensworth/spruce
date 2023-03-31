#include "StagingBufferBatch.h"
#include "vulkan_core.h"
#include <string>


namespace spr::gfx {

StagingBuffers::StagingBuffers(){}

StagingBuffers::StagingBuffers(VulkanResourceManager* rm) {
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

    m_totalOverflowSizeBytes = 0;
}

StagingBuffers::~StagingBuffers() {
    if (m_destroyed)
        return;
    
    SprLog::warn("[StagingBuffers] [~] Calling destroy() in destructor");
    destroy();
}

void StagingBuffers::destroy(){
    reset(); // remove any overflow

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
    m_destroyed = true;
}


Handle<Buffer> StagingBuffers::getStagingBuffer(uint32 sizeBytes) {
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
    m_totalOverflowSizeBytes += sizeBytes;

    return overflowStage;
}

void StagingBuffers::reset() {
    m_count64MB = 0;
    m_count16MB = 0;
    m_count4MB  = 0;
    m_count1MB  = 0;
    
    if (m_overflowStages.size() == 0)
        return;

    std::string msg("[StagingBuffers] Pre-allocated staging buffers exceeded, created: ");
    std::string info(std::to_string(m_overflowStages.size()) + " buffers, " + 
                     std::to_string(m_totalOverflowSizeBytes) + " bytes");
    SprLog::info(msg + info);

    // destroy and deallocate extra staging buffers
    for (Handle<Buffer> buffer : m_overflowStages){
        m_rm->remove(buffer);
    }
    m_overflowStages = std::vector<Handle<Buffer>>();
    m_totalOverflowSizeBytes = 0;
}

}