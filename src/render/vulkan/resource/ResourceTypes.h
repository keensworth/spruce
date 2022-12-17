#pragma once

#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "ResourceFlags.h"
#include "../core/spruce_core.h"
#include "../vulkan_core.h"
#include "glm/fwd.hpp"
#include "memory/Handle.h"

namespace spr::gfx {

// ------------------------------------------------------------------------- //
//                 Resource Implementations                                  // 
// ------------------------------------------------------------------------- //

typedef struct Buffer {
    VkBuffer buffer;
} Buffer;


typedef struct Texture {
    VkImage image;
    VkImageView view;
    VkSampler sampler;
} Texture;


// typedef struct FrameBuffer {
//     VkFramebuffer frameBuffer;
// } FrameBuffer;


typedef struct DescriptorSetLayout {
    VkDescriptorSetLayout descriptorSetLayout;
} DescriptorSetLayout;


typedef struct DescriptorSet {
    VkDescriptorSet descriptorSet;
} DescriptorSet;


typedef struct RenderPassLayout {
    VkAttachmentReference depthReference;
    std::vector<VkAttachmentReference> colorReferences;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    VkSubpassDescription subpassDescription;
    uint32 attachmentCount;
} RenderPassLayout;


typedef struct RenderPass {
    Handle<RenderPassLayout> layout;
    VkRenderPass renderPass;
} RenderPass;


typedef struct Shader {
    VkPipeline pipeline;
} Shader;



// ------------------------------------------------------------------------- //
//                 Resource Descriptions                                     // 
// ------------------------------------------------------------------------- //

typedef struct BufferDesc {
    static const uint32 ALL_BYTES = 0xFFFFFFFF;
    Handle<Buffer> buffer;
    uint32 byteOffset = 0;
    uint32 byteSize   = ALL_BYTES;
    uint32 usage      = Flags::BufferUsage::BU_UNIFORM_BUFFER;
} BufferDesc;


typedef struct TextureViewDesc {
    static const uint32 ALL_MIPS = 16;
    static const uint32 ALL_LAYERS = 16;

    typedef struct Sampler {
        uint32 minFilter  = Flags::Filter::NEAREST;
        uint32 magFilter  = Flags::Filter::NEAREST;
        uint32 mipmapMode = Flags::Mipmap::MODE_NEAREST;
        uint32 addressing = Flags::Wrap::REPEAT;
        float anisotropy  = 8.f;
        uint32 compare    = Flags::Compare::ALWAYS;
    } Sampler;

    typedef struct View {
        uint32 baseMip    = 0;
        uint32 mips       = ALL_MIPS;
        uint32 baseLayer  = 0;
        uint32 layers     = ALL_LAYERS;
    } View;

    Handle<Texture> texture;
    Sampler sampler;
    View view;
} TextureViewDesc;


typedef struct TextureDesc {
    static const uint32 ALL_MIPS = 16;
    static const uint32 ALL_LAYERS = 16;

    typedef struct Sampler {
        uint32 minFilter  = Flags::Filter::NEAREST;
        uint32 magFilter  = Flags::Filter::NEAREST;
        uint32 mipmapMode = Flags::Mipmap::MODE_NEAREST;
        uint32 addressing = Flags::Wrap::REPEAT;
        float anisotropy  = 8.f;
        uint32 compare    = Flags::Compare::ALWAYS;
    } Sampler;

    typedef struct View {
        uint32 baseMip    = 0;
        uint32 mips       = ALL_MIPS;
        uint32 baseLayer  = 0;
        uint32 layers     = ALL_LAYERS;
    } View;

    glm::vec3 dimensions {0,0,0};
    uint32 format  = Flags::Format::UNDEFINED_FORMAT;
    uint32 usage   = Flags::ImageUsage::IU_SAMPLED;
    uint32 samples = Flags::Sample::SAMPLE_1;
    Sampler sampler;
    View view;
} TextureDesc;


// typedef struct FrameBufferDesc { 
//     Handle<Texture> depthAttachment;
//     std::vector<Handle<Texture>> colorAttatchments;
//     Handle<RenderPass> renderPass;
//     glm::vec3 dimensions {0,0,0};
// } FrameBufferDesc;


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

    std::vector<TextureBindingLayout> textures;
    std::vector<BufferBindingLayout> buffers;
} DescriptorSetLayoutDesc;


typedef struct DescriptorSetDesc {
    static const uint32 ALL_BYTES = 0xFFFFFFFF;

    typedef struct TextureBinding{
        Handle<Texture> texture;
        TextureDesc::Sampler sampler; // override
        TextureDesc::View view;   // overried
    } TextureBinding;

    typedef struct BufferBinding {
        Handle<Buffer> buffer;
        uint32 byteOffset = 0;
        uint32 byteSize   = ALL_BYTES;
    } BufferBinding;

    std::vector<TextureBinding> textures;
    std::vector<BufferBinding> buffers;
    Handle<DescriptorSetLayout> layout;
} DescriptorSetDesc;


typedef struct RenderPassLayoutDesc { 
    typedef struct SubpassLayout {
        bool depthAttachment = false;
        std::vector<uint32> colorAttachments;
    } SubpassLayout;

    uint32 depthAttachmentFormat = Flags::Format::UNDEFINED_FORMAT;
    std::vector<uint32> colorAttatchmentFormats;
    SubpassLayout subpass;
} RenderPassLayoutDesc;


typedef struct RenderPassDesc { 
    typedef struct Attachment {
        Handle<Texture> texture; // framebuffer dst only
        uint32 format         = Flags::Format::UNDEFINED_FORMAT;
        uint32 samples        = Flags::Sample::SAMPLE_1;
        uint32 loadOp         = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 storeOp        = Flags::StoreOp::STORE_DONT_CARE;
        uint32 stencilLoadOp  = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 stencilStoreOp = Flags::StoreOp::STORE_DONT_CARE;
        uint32 layout         = Flags::ImageLayout::UNDEFINED;
        uint32 finalLayout    = Flags::ImageLayout::UNDEFINED;
    } Attachment;

    Attachment depthAttachment;
    std::vector<Attachment> colorAttachments;
    Handle<RenderPassLayout> layout;
} RenderPassDesc;


typedef struct ShaderDesc {
    typedef struct Shader{
        std::vector<uint8> byteCode;
    } Shader;

    typedef struct GraphicsState {
        Flags::Compare depthTest = Flags::Compare::ALWAYS;
        Handle<RenderPass> renderPass;
    } GraphicsState;

    Shader vertexShader;
    Shader fragmentShader;
    Shader computeShader;
    std::vector<Handle<DescriptorSetLayout>> descriptorSets;

} ShaderDesc;





}