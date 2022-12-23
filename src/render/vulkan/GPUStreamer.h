#pragma once

#include "spruce_core.h"
#include "memory/Handle.h"
#include "memory/TempBuffer.h"
#include "resource/ResourceTypes.h"

namespace spr::gfx {
class GPUStreamer {
public:
    GPUStreamer();
    ~GPUStreamer();
    
    void upload(TempBuffer src, Handle<Buffer> dst);
    void uploadDynamic(TempBuffer src, Handle<Buffer> dst, uint32 frame);

private:


};
}