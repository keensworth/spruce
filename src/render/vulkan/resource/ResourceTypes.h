#pragma once

#include <string>
#include <typeindex>
#include <typeinfo>
#include <span>
#include "ResourceFlags.h"
#include "../core/spruce_core.h"
#include "glm/fwd.hpp"
#include "memory/Handle.h"

namespace spr::gfx {

// resource implementations
typedef struct Shader {

} Shader;

typedef struct Pipeline {

} Pipeline;

typedef struct Buffer {

} Buffer;

typedef struct Texture {

} Texture;

typedef struct Mesh {

} Mesh;

typedef struct DescriptorSetLayout {

} DescriptorSetLayout;

typedef struct DescriptorSet {

} DescriptorSet;

typedef struct RenderPassLayout {

} RenderPassLayout;

typedef struct RenderPass {

} RenderPass;

typedef struct FrameBuffer {

} FrameBuffer;



// resource descriptions
typedef struct PipelineDesc {

} PipelineDesc;


typedef struct MeshDesc {

} MeshDesc;


typedef struct DescriptorSetLayoutDesc {
    typedef struct TextureBindingLayout {
        uint32 slot   = 0;
        uint32 stages = Flags::DescriptorStage::VERTEX  |
                        Flags::DescriptorStage::FRAGMENT;
        uint32 type   = Flags::DescriptorType::COMBINED_IMAGE_SAMPLER;
    } TextureBindingLayout;

    typedef struct BufferBindingLayout {
        uint32 slot   = 0;
        uint32 stages = Flags::DescriptorStage::VERTEX  |
                        Flags::DescriptorStage::FRAGMENT;
        uint32 type   = Flags::DescriptorType::UNIFORM_BUFFER;
    } BufferBindingLayout;

    std::span<TextureBindingLayout> textures;
    std::span<BufferBindingLayout> buffers;
} DescriptorSetLayoutDesc;


typedef struct DescriptorSetDesc {
     typedef struct TextureBinding{
        Handle<Texture> texture = Handle<Texture>();
        //Handle<Sampler> sampler = Handle<Sampler>();
        //Handle<View> view = Handle<View>();
        uint32 format     = Flags::Format::RGBA8_UNORM;
        uint32 baseMip    = 0;
        uint32 mips       = 0xFFFFFFFF;
        uint32 baseLayer  = 0;
        uint32 layerCount = 0xFFFFFFFF;
        uint32 slot       = 0;
    } TextureBinding;

    typedef struct BufferBinding {
        Handle<Buffer> buffer = Handle<Buffer>();
        uint32 offset = 0;
        uint32 size   = 0xFFFFFFFF;
        uint32 slot   = 0;
    } BufferBinding;

    std::span<TextureBinding> textures;
    std::span<BufferBinding> buffers;
    Handle<DescriptorSetLayout> layout = Handle<DescriptorSetLayout>();
} DescriptorSetDesc;


typedef struct RenderPassLayoutDesc {
    typedef struct SubpassLayout {
        bool depthAttachment   = false;
        bool subpassDepthInput = false;
        std::span<uint32> colorAttachments;
        std::span<uint32> subpassInputs;
    } SubpassLayout;

    uint32 depthAttachmentFormat = Flags::Format::UNDEFINED_FORMAT;
    std::span<uint32> colorAttatchmentFormats;
    std::span<SubpassLayout> subpasses;
} RenderPassLayoutDesc;


typedef struct RenderPassDesc {
    typedef struct Attachment {
        uint32 format         = Flags::Format::UNDEFINED_FORMAT;
        uint32 samples        = Flags::Sample::SAMPLE_1;
        uint32 loadOp         = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 storeOp        = Flags::StoreOp::STORE_DONT_CARE;
        uint32 stencilLoadOp  = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 stencilStoreOp = Flags::StoreOp::STORE_DONT_CARE;
        uint32 usage          = Flags::ImageLayout::UNDEFINED;
        uint32 nextUsage      = Flags::ImageLayout::UNDEFINED;
    } Attachment;

    std::span<Attachment> attachments;
    Handle<RenderPassLayout> layout = Handle<RenderPassLayout>();
} RenderPassDesc;



typedef struct BufferDesc {

} BufferDesc;


typedef struct TextureDesc {
    glm::vec3 dimensions;
    uint32 mips;
    uint32 format;
    uint32 usage;
    
} TextureDesc;


typedef struct FrameBufferDesc {
    Handle<Texture> depthAttachment = Handle<Texture>();
    std::span<Handle<Texture>> colorAttatchments;
    Handle<RenderPass> renderPass = Handle<RenderPass>();
    glm::vec3 dimensions {0,0,0};
} FrameBufferDesc;


typedef struct ShaderDesc {
    typedef struct Shader{
        std::span<uint8> byteCode;
    } Shader;

    Shader vertexShader;
    Shader fragmentShader;
    Shader computeShader;

    std::span<DescriptorSetLayoutDesc> descriptorSets;
} ShaderDesc;





}