#include "UploadHandler.h"
#include "GPUStreamer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"

namespace spr::gfx {

UploadHandler::UploadHandler() {
    
}

UploadHandler::UploadHandler(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer)
                            : m_streamer(device, rm, transferCommandBuffer, graphicsCommandBuffer) {
    m_transferCommandBuffer = &transferCommandBuffer;
    m_graphicsCommandBuffer = &graphicsCommandBuffer;
    reset();
}

UploadHandler::~UploadHandler() {
    // teardown GPUStreamer + associated staging buffers
    m_streamer.~GPUStreamer();
}

void UploadHandler::submit() {
    // upload buffers
    for (GPUStreamer::BufferTransfer upload : m_bufferUploadQueue) {
        m_streamer.transfer(upload);
    }

    // upload dynamic buffers (N-buffered)
    for (GPUStreamer::BufferTransfer upload : m_dynamicBufferUploadQueue) {
        m_streamer.transferDynamic(upload, m_frameId);
    }

    // upload textures/images
    for (GPUStreamer::TextureTransfer upload : m_textureUploadQueue) {
        m_streamer.transfer(upload);
    }

    // flush uploads and submit transfer command buffer
    m_streamer.flush();
    m_transferCommandBuffer->submit();

    // prepare gfx resource barriers (but don't submit)
    m_graphicsCommandBuffer->begin();
    m_streamer.performGraphicsBarriers();
    m_graphicsCommandBuffer->end();
}

void UploadHandler::setFrameId(uint32 frame){
    m_frameId = frame;
}

void UploadHandler::reset() {
    m_bufferUploadQueue = std::vector<GPUStreamer::BufferTransfer>();
    m_bufferUploadQueue.reserve(32);
    m_dynamicBufferUploadQueue = std::vector<GPUStreamer::BufferTransfer>();
    m_dynamicBufferUploadQueue.reserve(32);
    m_textureUploadQueue = std::vector<GPUStreamer::TextureTransfer>();
    m_textureUploadQueue.reserve(32);
}

}