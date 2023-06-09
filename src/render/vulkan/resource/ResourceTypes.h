#pragma once

#include <cmath>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <vector>
#include "../../external/volk/volk.h"
#include "ResourceFlags.h"
#include "../core/spruce_core.h"
#include "../gfx_vulkan_core.h"
#include "glm/fwd.hpp"
#include "memory/Handle.h"
#include <vk_mem_alloc.h>
#include <span>
#include <vulkan/vulkan_core.h>

namespace spr::gfx {

typedef enum MemoryType : uint32{
    DEVICE = 1,
    HOST   = 2
} MemoryType;



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
    uint32 memType;
    VmaAllocation alloc;
    VmaAllocationInfo allocInfo; // TODO remove
    struct Desc;
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
    glm::uvec3 dimensions;
    uint32 mips;
    uint32 layers;
    VkSampleCountFlagBits samples;
    VkImageUsageFlags usage;
    VkImageSubresourceRange subresourceRange;
    bool defaultRes = true;
    struct Desc;
} Texture;


// --------------------------------------------------------- //
//                 Texture Attachment                        // 
// --------------------------------------------------------- //
typedef struct TextureAttachment {
    std::vector<Handle<Texture>> textures;
    struct Desc;
} TextureAttachment;


// --------------------------------------------------------- //
//                 Descriptor Set Layout                     // 
// --------------------------------------------------------- //
typedef struct DescriptorSetLayout {
    typedef struct TextureBindingLayout {
        uint32 binding = 0;
        uint32 stages  = Flags::DescriptorStage::FRAGMENT;
        uint32 type    = Flags::DescriptorType::COMBINED_IMAGE_SAMPLER;
        uint32 count   = 1; // only != 1 if using array of textures
    } TextureBindingLayout;

    typedef struct BufferBindingLayout {
        uint32 binding = 0;
        uint32 stages  = Flags::DescriptorStage::VERTEX  |
                         Flags::DescriptorStage::FRAGMENT;
        uint32 type    = Flags::DescriptorType::UNIFORM_BUFFER;
    } BufferBindingLayout;     

    std::vector<TextureBindingLayout> textureLayouts{};
    std::vector<BufferBindingLayout> bufferLayouts{};
    VkDescriptorSetLayout descriptorSetLayout;
    struct Desc;
} DescriptorSetLayout;


// --------------------------------------------------------- //
//                 Descriptor Set                            // 
// --------------------------------------------------------- //
typedef struct DescriptorSet {
    //  global ==> (size == 1)
    // !global ==> (size == MAX_FRAME_COUNT)
    std::vector<VkDescriptorSet> descriptorSets{};
    bool global = false;
    struct Desc;
} DescriptorSet;


// --------------------------------------------------------- //
//                 Framebuffer                               // 
// --------------------------------------------------------- //
typedef struct Framebuffer {
    typedef struct ColorAttachment {
        Handle<TextureAttachment> texture;

        // swapchain image views (optional)
        // if set, will neglect attachment handle above
        std::vector<VkImageView> swapchainImageViews;

        // properties
        uint32 format         = Flags::Format::UNDEFINED_FORMAT;
        uint32 samples        = Flags::Sample::SAMPLE_1;
        uint32 loadOp         = Flags::LoadOp::LOAD_CLEAR;
        uint32 storeOp        = Flags::StoreOp::STORE;
        uint32 stencilLoadOp  = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 stencilStoreOp = Flags::StoreOp::STORE_DONT_CARE;
        uint32 layout         = Flags::ImageLayout::UNDEFINED;
        uint32 finalLayout    = Flags::ImageLayout::READ_ONLY;
    } ColorAttachment;

    typedef struct DepthAttachment {
        Handle<TextureAttachment> texture;
        uint32 format         = Flags::Format::UNDEFINED_FORMAT;
        uint32 samples        = Flags::Sample::SAMPLE_1;
        uint32 loadOp         = Flags::LoadOp::LOAD_CLEAR;
        uint32 storeOp        = Flags::StoreOp::STORE;
        uint32 stencilLoadOp  = Flags::LoadOp::LOAD_DONT_CARE;
        uint32 stencilStoreOp = Flags::StoreOp::STORE_DONT_CARE;
        uint32 layout         = Flags::ImageLayout::UNDEFINED;
        uint32 finalLayout    = Flags::ImageLayout::ATTACHMENT;
        // overwritten by create<Shader>, not to be set
        uint32 compareOp;
    } DepthAttachment;

    std::vector<VkFramebuffer> framebuffers;
    glm::uvec3 dimensions;

    bool hasDepthAttachment = false;
    DepthAttachment depthAttachment;
    std::vector<ColorAttachment> colorAttachments;
    bool swapchainOverride = false;

    struct Desc;
} Framebuffer;


// --------------------------------------------------------- //
//                 Render Pass Layout                        // 
// --------------------------------------------------------- //
typedef struct RenderPassLayout {
    VkAttachmentReference depthReference;
    std::vector<VkAttachmentReference> colorReferences;
    std::vector<VkAttachmentDescription> attachmentDescriptions;

    uint32 colorAttachmentCount = 0;
    bool   depthAttachmnentCount = 0;

    struct Desc;
} RenderPassLayout;


// --------------------------------------------------------- //
//                 Render Pass                               // 
// --------------------------------------------------------- //
typedef struct RenderPass {
    Handle<RenderPassLayout> layout;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    Handle<Framebuffer> framebuffer;
    glm::uvec3 dimensions;
    uint32 samples;
    struct Desc;
} RenderPass;


// --------------------------------------------------------- //
//                 Shader                                    // 
// --------------------------------------------------------- //
typedef struct Shader {
    VkPipelineLayout layout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    std::vector<VkDescriptorSetLayout> emptyDescSetLayouts;
    VkShaderModule vertexModule = VK_NULL_HANDLE;
    VkShaderModule fragmentModule = VK_NULL_HANDLE;
    struct Desc;
    
    typedef struct GraphicsState {
        Flags::Compare depthTest = Flags::Compare::ALWAYS;
        bool depthTestEnabled = true;
        bool depthWriteEnabled = true;
        Handle<RenderPass> renderPass;
    } GraphicsState;

    std::string vertexPath = "";
    std::string fragmentPath = "";
    std::vector<Handle<DescriptorSetLayout>> descSetLayouts;
    GraphicsState graphicsState;
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
typedef struct Buffer::Desc {
    uint32 byteOffset  = 0;
    uint32 byteSize    = 0;
    uint32 usage       = Flags::BufferUsage::BU_UNIFORM_BUFFER;
    uint32 memType = DEVICE;
} BufferDesc;


// --------------------------------------------------------- //
//                 Texture Desc                              // 
// --------------------------------------------------------- //
typedef struct Texture::Desc {
    static const uint32 ALL_MIPS = 16;
    static const uint32 ALL_LAYERS = 16;

    typedef struct Sampler {
        uint32 minFilter  = Flags::Filter::LINEAR;
        uint32 magFilter  = Flags::Filter::LINEAR;
        uint32 mipmapMode = Flags::Mipmap::MODE_LINEAR;
        uint32 addressing = Flags::Wrap::REPEAT;
        float anisotropy  = 8.f;
        uint32 compare    = Flags::Compare::ALWAYS;
    } Sampler;

    typedef struct View {
        uint32 baseMip    = 0;
        uint32 mips       = 1;//ALL_MIPS;
        uint32 baseLayer  = 0;
        uint32 layers     = 1;//ALL_LAYERS;
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
typedef struct TextureAttachment::Desc {
    TextureDesc textureLayout;
} TextureAttachmentDesc;


// --------------------------------------------------------- //
//                 Descriptor Set Layout Desc                // 
// --------------------------------------------------------- //
typedef struct DescriptorSetLayout::Desc {
    typedef DescriptorSetLayout::TextureBindingLayout TextureBindingLayout;
    typedef DescriptorSetLayout::BufferBindingLayout BufferBindingLayout;
    
    std::vector<TextureBindingLayout> textures{};
    std::vector<BufferBindingLayout> buffers{};
} DescriptorSetLayoutDesc;


// --------------------------------------------------------- //
//                 Descriptor Set Desc                       // 
// --------------------------------------------------------- //
typedef struct DescriptorSet::Desc {
    static const uint32 ALL_BYTES = 0xFFFFFFFF;
    
    typedef struct TextureBinding {            // [mutually exclusive]
        Handle<Texture> texture;               // global texture
        std::vector<Handle<Texture>> textures; // global array of textures
        Handle<TextureAttachment> attachment;  // per-frame textures
        // default layout, will override all entries
        Flags::ImageLayout layout = Flags::ImageLayout::READ_ONLY;
    } TextureBinding;

    typedef struct BufferBinding {           // [mutually exclusive]
        Handle<Buffer> buffer;               // global buffer
        std::vector<Handle<Buffer>> buffers; // per-frame buffers

        uint32 byteOffset = 0;
        uint32 byteSize   = ALL_BYTES;
    } BufferBinding;

    std::vector<TextureBinding> textures;
    std::vector<BufferBinding> buffers;
    Handle<DescriptorSetLayout> layout;
} DescriptorSetDesc;


// --------------------------------------------------------- //
//                 Framebuffer Desc                          // 
// --------------------------------------------------------- //
typedef struct Framebuffer::Desc {
    glm::uvec3 dimensions = {0,0,0};
    Handle<RenderPass> renderPass;

    bool hasDepthAttachment = 0;
    DepthAttachment depthAttachment;
    std::vector<ColorAttachment> colorAttachments{};

    bool swapchainOverride = false;
} FramebufferDesc;


// --------------------------------------------------------- //
//                 Render Pass Layout Desc                   // 
// --------------------------------------------------------- //
typedef struct RenderPassLayout::Desc { 
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
typedef struct RenderPass::Desc { 
    typedef Framebuffer::DepthAttachment DepthAttachment;
    typedef Framebuffer::ColorAttachment ColorAttachment;

    glm::uvec3 dimensions = {0,0,0};
    Handle<RenderPassLayout> layout;
    
    DepthAttachment depthAttachment;
    std::vector<ColorAttachment> colorAttachments{};
} RenderPassDesc;


// --------------------------------------------------------- //
//                 Shader Desc                               // 
// --------------------------------------------------------- //
typedef struct Shader::Desc {
    typedef struct Shader{
        std::string path = "";
    } Shader;

    typedef struct GraphicsState {
        Flags::Compare depthTest = Flags::Compare::ALWAYS;
        bool depthTestEnabled = true;
        bool depthWriteEnabled = true;
        Handle<RenderPass> renderPass;
    } GraphicsState;

    Shader vertexShader;
    Shader fragmentShader;
    // Shader computeShader;
    std::vector<Handle<DescriptorSetLayout>> descriptorSets;
    GraphicsState graphicsState;
} ShaderDesc;

}