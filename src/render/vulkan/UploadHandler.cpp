#include "UploadHandler.h"
#include "GPUStreamer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"
#include "../../debug/SprLog.h"

namespace spr::gfx {

UploadHandler::UploadHandler() {
    
}

UploadHandler::~UploadHandler() {
    if (m_destroyed || !m_initialized)
        return;
    
    SprLog::warn("[UploadHandler] [~] Calling destroy() in destructor");
    destroy();
}

UploadHandler& UploadHandler::operator=(UploadHandler&& other) noexcept{
    if (this != &other) {
        m_streamer = std::move(other.m_streamer);
        m_frameId = other.m_frameId;
        m_rm = other.m_rm;
        m_transferCommandBuffer = other.m_transferCommandBuffer;
        m_graphicsCommandBuffer = other.m_graphicsCommandBuffer;
        m_bufferUploadQueue = std::move(other.m_bufferUploadQueue);
        m_dynamicBufferUploadQueue = std::move(other.m_dynamicBufferUploadQueue);
        m_textureUploadQueue = std::move(other.m_textureUploadQueue);
        m_destroyed = other.m_destroyed;

        other.m_frameId = 0;
        other.m_rm = nullptr;
        other.m_transferCommandBuffer = nullptr;
        other.m_graphicsCommandBuffer = nullptr;
        other.m_bufferUploadQueue.clear();
        other.m_dynamicBufferUploadQueue.clear();
        other.m_textureUploadQueue.clear();
        other.m_destroyed = false;
    }
    return *this;
}

UploadHandler& UploadHandler::operator=(const UploadHandler& other) {
    if (this != &other) {
        m_streamer = other.m_streamer;
        m_frameId = other.m_frameId;
        m_rm = other.m_rm;
        m_transferCommandBuffer = other.m_transferCommandBuffer;
        m_graphicsCommandBuffer = other.m_graphicsCommandBuffer;
        m_bufferUploadQueue = other.m_bufferUploadQueue;
        m_dynamicBufferUploadQueue = other.m_dynamicBufferUploadQueue;
        m_textureUploadQueue = other.m_textureUploadQueue;
        m_destroyed = other.m_destroyed;
    }
    return *this;
}

void UploadHandler::init(VulkanDevice& device, VulkanResourceManager& rm, CommandBuffer& transferCommandBuffer, CommandBuffer& graphicsCommandBuffer){
    m_rm = &rm;
    m_transferCommandBuffer = &transferCommandBuffer;
    m_graphicsCommandBuffer = &graphicsCommandBuffer;
    reset();

    m_streamer.init(device, rm, transferCommandBuffer, graphicsCommandBuffer);

    m_initialized = true;
}

void UploadHandler::destroy(){
    m_streamer.destroy();
    reset();
    m_destroyed = true;
    SprLog::info("[UploadHandler] [destroy] destroyed...");
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
}

void UploadHandler::performGraphicsBarriers(){
    m_streamer.performGraphicsBarriers();
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
    m_streamer.reset();
}

}