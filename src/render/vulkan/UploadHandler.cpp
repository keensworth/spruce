#include "UploadHandler.h"
#include "GPUStreamer.h"
#include "vulkan/CommandBuffer.h"
#include "resource/ResourceTypes.h"
#include "resource/VulkanResourceManager.h"
#include "debug/SprLog.h"

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
        m_destroyed = other.m_destroyed;

        other.m_frameId = 0;
        other.m_rm = nullptr;
        other.m_transferCommandBuffer = nullptr;
        other.m_graphicsCommandBuffer = nullptr;
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
    m_streamer.reset();
}

}