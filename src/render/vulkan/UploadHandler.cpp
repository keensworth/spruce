#include "UploadHandler.h"
#include "GPUStreamer.h"
#include "resource/ResourceTypes.h"

namespace spr::gfx {

UploadHandler::UploadHandler() {
    
}

UploadHandler::UploadHandler(VulkanDevice& device) : m_streamer(device) {
    resetQueues();
}

UploadHandler::~UploadHandler() {

}

void UploadHandler::uploadBuffer(TempBuffer<uint8> src, Handle<Buffer> dst) {
    m_bufferUploadQueue.push_back({
        .src = src,
        .dst = dst
    });
}

void UploadHandler::uploadDyanmicBuffer(TempBuffer<uint8> src, Handle<Buffer> dst) {
    m_dynamicBufferUploadQueue.push_back({
        .src = src,
        .dst = dst
    });
}

void UploadHandler::uploadTexture(TempBuffer<uint8> src, Handle<Texture> dst) {
    m_textureUploadQueue.push_back({
        .src = src,
        .dst = dst
    });
}

void UploadHandler::submit(uint32 frame) {
    // upload buffers
    for (BufferUpload upload : m_bufferUploadQueue) {
        m_streamer.upload(upload.src, upload.dst);
    }

    // upload dynamic buffers (N-buffered)
    for (BufferUpload upload : m_dynamicBufferUploadQueue) {
        m_streamer.uploadDynamic(upload.src, upload.dst, frame);
    }

    // upload textures/images
    for (TextureUpload upload : m_textureUploadQueue) {
        m_streamer.upload(upload.src, upload.dst);
    }

    resetQueues();
}

void UploadHandler::resetQueues() {
    m_bufferUploadQueue = std::vector<BufferUpload>();
    m_bufferUploadQueue.reserve(32);
    m_dynamicBufferUploadQueue = std::vector<BufferUpload>();
    m_dynamicBufferUploadQueue.reserve(32);
    m_textureUploadQueue = std::vector<TextureUpload>();
    m_textureUploadQueue.reserve(32);
}

}