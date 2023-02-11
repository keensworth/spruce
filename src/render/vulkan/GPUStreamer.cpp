#include "GPUStreamer.h"

namespace spr::gfx {

GPUStreamer::GPUStreamer() {
    
}

GPUStreamer::GPUStreamer(VulkanDevice& device){
    m_device = &device;
}

GPUStreamer::~GPUStreamer(){

}

template<>
void GPUStreamer::upload(TempBuffer<uint8> src, Handle<Buffer> dst) {
    // (graphics) release ownership of buffer (maybe the upload hander will take care of rel/acq)
    // (transfer) acquire ownership of buffer

    // if (dst == shared) ? {copy to shared} : {copy to staging + copy command}

    // (transfer) release ownership of buffer
    // (graphics) acquire ownership of buffer
}

template<>
void GPUStreamer::upload(TempBuffer<uint8> src, Handle<Texture> dst) {
    // (graphics) release ownership of buffer (maybe the upload hander will take care of rel/acq)
    // (transfer) acquire ownership of buffer

    // if (dst == shared) ? {copy to shared} : {copy to staging + copy command}

    // (transfer) release ownership of buffer
    // (graphics) acquire ownership of buffer
}

template<>
void GPUStreamer::uploadDynamic(TempBuffer<uint8> src, Handle<Buffer> dst, uint32 frame) {
    // (graphics) release ownership of buffer
    // (transfer) acquire ownership of buffer

    // if (dst == shared) ? {copy to shared} : {copy to staging + copy command} (index into N-buffer w/ frame)

    // (transfer) release ownership of buffer
    // (graphics) acquire ownership of buffer
}

}
