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
#include "vk_mem_alloc.h"

namespace spr::gfx {



//  ██╗███╗   ███╗██████╗ ██╗     
//  ██║████╗ ████║██╔══██╗██║     
//  ██║██╔████╔██║██████╔╝██║     
//  ██║██║╚██╔╝██║██╔═══╝ ██║     
//  ██║██║ ╚═╝ ██║██║     ███████╗
//  ╚═╝╚═╝     ╚═╝╚═╝     ╚══════╝

// --------------------------------------------------------- //
//                 Buffer                                    // 
// --------------------------------------------------------- //

typedef struct Buffer {
    uint32 byteSize = 0;
    VkBuffer buffer;
    VmaAllocation alloc;
    VmaAllocationInfo allocInfo;
    bool hostVisible = false;
} Buffer;


// --------------------------------------------------------- //
//                 Texture                                   // 
// --------------------------------------------------------- //

typedef struct Texture {
    VkImage image;
    VkImageView view;
    VkSampler sampler;
    VmaAllocation alloc;
    VmaAllocationInfo allocInfo;
    VkFormat format;
    glm::vec3 dimensions;
    uint32 mips;
    uint32 layers;
    VkSampleCountFlagBits samples;
    VkImageUsageFlags usage;
    VkImageSubresourceRange subresourceRange;
    bool defaultRes = true;
} Texture;


// --------------------------------------------------------- //
//                 Texture Attachment                        // 
// --------------------------------------------------------- //

typedef struct TextureAttachment {
    std::vector<Handle<Texture>> textures;
} TextureAttachment;


// --------------------------------------------------------- //
//                 Descriptor Set Layout                     // 
// --------------------------------------------------------- //

typedef struct DescriptorSetLayout {
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

    std::vector<TextureBindingLayout> textureLayouts;
    std::vector<BufferBindingLayout> bufferLayouts;
    VkDescriptorSetLayout descriptorSetLayout;
} DescriptorSetLayout;


// --------------------------------------------------------- //
//                 Descriptor Set                            // 
// --------------------------------------------------------- //

typedef struct DescriptorSet {
    bool dynamic = false;
    std::vector<VkDescriptorSet> descriptorSets;
} DescriptorSet;


// --------------------------------------------------------- //
//                 Render Pass Layout                        // 
// --------------------------------------------------------- //

typedef struct RenderPassLayout {
    VkAttachmentReference depthReference;
    std::vector<VkAttachmentReference> colorReferences;
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    VkSubpassDescription subpassDescription;
    uint32 attachmentCount;
} RenderPassLayout;


// --------------------------------------------------------- //
//                 Render Pass                               // 
// --------------------------------------------------------- //

typedef struct RenderPass {
    Handle<RenderPassLayout> layout;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;
    glm::uvec3 dimensions;
    uint32 samples;

    // only required to set additional properties or texture handle
    typedef struct ColorAttachment {
        Handle<TextureAttachment> texture; // framebuffer 
        uint32 format         = Flags::Format::UNDEFINED_FORMAT;
        uint32 samples        = Flags::Sample::SAMPLE_1;
        uint32 loadOp         = Flags::LoadOp::LOAD_CLEAR;
        uint32 storeOp        = Flags::StoreOp::STORE;
        uint32 stencilLoadOp  = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 stencilStoreOp = Flags::StoreOp::STORE_DONT_CARE;
        uint32 layout         = Flags::ImageLayout::UNDEFINED;
        uint32 finalLayout    = Flags::ImageLayout::COLOR_ATTACHMENT;
    } ColorAttachment;

    typedef struct DepthAttachment {
        Handle<TextureAttachment> texture; // framebuffer 
        uint32 format         = Flags::Format::UNDEFINED_FORMAT;
        uint32 samples        = Flags::Sample::SAMPLE_1;
        uint32 loadOp         = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 storeOp        = Flags::StoreOp::STORE_DONT_CARE;
        uint32 stencilLoadOp  = Flags::LoadOp::LOAD_CLEAR;
        uint32 stencilStoreOp = Flags::StoreOp::STORE;
        uint32 layout         = Flags::ImageLayout::UNDEFINED;
        uint32 finalLayout    = Flags::ImageLayout::DEPTH_STENCIL_ATTACHMENT;
    } DepthAttachment;

    bool hasDepthAttachment = false;
    DepthAttachment depthAttachment;
    std::vector<ColorAttachment> colorAttachments;
} RenderPass;


// --------------------------------------------------------- //
//                 Shader                                    // 
// --------------------------------------------------------- //

typedef struct Shader {
    VkPipeline pipeline;
} Shader;




//  ██████╗ ███████╗ ██████╗ █████╗ 
//  ██╔══██╗██╔════╝██╔════╝██╔══██╗
//  ██║  ██║█████╗  ╚█████╗ ██║  ╚═╝
//  ██║  ██║██╔══╝   ╚═══██╗██║  ██╗
//  ██████╔╝███████╗██████╔╝╚█████╔╝
//  ╚═════╝ ╚══════╝╚═════╝  ╚════╝ 


// --------------------------------------------------------- //
//                 Buffer Desc                               // 
// --------------------------------------------------------- //

typedef struct BufferDesc {
    uint32 byteOffset = 0;
    uint32 byteSize   = 0;
    uint32 usage      = Flags::BufferUsage::BU_UNIFORM_BUFFER;
    bool hostVisible  = false;
} BufferDesc;


// --------------------------------------------------------- //
//                 Texture Desc                              // 
// --------------------------------------------------------- //

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

    glm::uvec3 dimensions {0,0,0};
    uint32 format  = Flags::Format::UNDEFINED_FORMAT;
    uint32 usage   = Flags::ImageUsage::IU_SAMPLED;
    uint32 samples = Flags::Sample::SAMPLE_1;
    Sampler sampler;
    View view;
} TextureDesc;


// --------------------------------------------------------- //
//                 Texture Attachment Desc                   // 
// --------------------------------------------------------- //

typedef struct TextureAttachmentDesc {
    TextureDesc textureLayout;
} TextureAttachmentDesc;


// --------------------------------------------------------- //
//                 Descriptor Set Layout Desc                // 
// --------------------------------------------------------- //

typedef struct DescriptorSetLayoutDesc {
    std::vector<DescriptorSetLayout::TextureBindingLayout> textures;
    std::vector<DescriptorSetLayout::BufferBindingLayout> buffers;
} DescriptorSetLayoutDesc;


// --------------------------------------------------------- //
//                 Descriptor Set Desc                       // 
// --------------------------------------------------------- //

typedef struct DescriptorSetDesc {
    static const uint32 ALL_BYTES = 0xFFFFFFFF;

    typedef struct TextureBinding{
        Handle<Texture> texture;
        TextureDesc::Sampler sampler; // override
        TextureDesc::View view;   // override
    } TextureBinding;

    typedef struct BufferBinding {
        Handle<Buffer> buffer;
        uint32 byteOffset = 0;
        uint32 byteSize   = ALL_BYTES;
    } BufferBinding;

    bool dynamic = false;
    std::vector<TextureBinding> textures;
    std::vector<BufferBinding> buffers;
    Handle<DescriptorSetLayout> layout;
} DescriptorSetDesc;


// --------------------------------------------------------- //
//                 Render Pass Layout Desc                   // 
// --------------------------------------------------------- //

typedef struct RenderPassLayoutDesc { 
    typedef struct SubpassLayout {
        bool depthAttachment = false;
        std::vector<uint32> colorAttachments;
    } SubpassLayout;

    uint32 depthAttachmentFormat = Flags::Format::UNDEFINED_FORMAT;
    std::vector<uint32> colorAttatchmentFormats;
    SubpassLayout subpass;
} RenderPassLayoutDesc;


// --------------------------------------------------------- //
//                 Render Pass Desc                          // 
// --------------------------------------------------------- //

typedef struct RenderPassDesc { 
    glm::uvec3 dimensions = {0,0,0};
    Handle<RenderPassLayout> layout;
    RenderPass::DepthAttachment depthAttachment;
    std::vector<RenderPass::ColorAttachment> colorAttachments;
} RenderPassDesc;


// --------------------------------------------------------- //
//                 Shader Desc                               // 
// --------------------------------------------------------- //

typedef struct ShaderDesc {
    typedef struct Shader{
        std::string shaderPath = "";
    } Shader;

    typedef struct GraphicsState {
        Flags::Compare depthTest = Flags::Compare::ALWAYS;
        Handle<RenderPass> renderPass;
    } GraphicsState;

    Shader vertexShader;
    Shader fragmentShader;
    // Shader computeShader;
    std::vector<Handle<DescriptorSetLayout>> descriptorSets;
    GraphicsState graphicsState;
} ShaderDesc;

}