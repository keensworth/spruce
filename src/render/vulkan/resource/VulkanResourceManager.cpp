#include "VulkanResourceManager.h"

#include "ResourceFlags.h"
#include "ResourceTypes.h"
#include <filesystem>
#include <vulkan/vulkan_core.h>
#include <fstream>
#include "vk_mem_alloc.h"


namespace spr::gfx {


//  ██╗███╗  ██╗██╗████████╗
//  ██║████╗ ██║██║╚══██╔══╝
//  ██║██╔██╗██║██║   ██║   
//  ██║██║╚████║██║   ██║   
//  ██║██║ ╚███║██║   ██║   
//  ╚═╝╚═╝  ╚══╝╚═╝   ╚═╝   

VulkanResourceManager::VulkanResourceManager(){

}

void VulkanResourceManager::init(VulkanDevice& device, VmaAllocator& allocator){
    // set device and allocator
    m_device = device.getDevice();
    m_allocator = &allocator;

    // setup global descriptor pool
    std::vector<VkDescriptorPoolSize> globalPoolSizes = {
        {
            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 4
        }
    };
    VkDescriptorPoolCreateInfo globalPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = (uint32)globalPoolSizes.size(),
        .pPoolSizes = globalPoolSizes.data()
    };
    VK_CHECK(vkCreateDescriptorPool(m_device, &globalPoolInfo, NULL, &m_globalDescriptorPool));

    // setup dynamic descriptor pools
    for (int frame = 0; frame < MAX_FRAME_COUNT; frame++){
        std::vector<VkDescriptorPoolSize> dynamicPoolSizes = {
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 16
            },
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 16
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 16
            },
        };
        VkDescriptorPoolCreateInfo dynamicPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .maxSets = 1024,
            .poolSizeCount = (uint32)dynamicPoolSizes.size(),
            .pPoolSizes = dynamicPoolSizes.data()
        };
        VK_CHECK(vkCreateDescriptorPool(m_device, &dynamicPoolInfo, NULL, &m_dynamicDescriptorPools[frame]));
    }
    
}

VulkanResourceManager::~VulkanResourceManager(){
    // delete buffer cache
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
    delete bufferCache;

    // delete texture cache
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    delete textureCache;

    // delete descriptor set layout cache
    DescriptorSetLayoutCache* descriptorSetLayoutCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
    delete descriptorSetLayoutCache;

    // delete descriptor set cache
    DescriptorSetCache* descriptorSet = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
    delete descriptorSet;

    // delete render pass layout cache
    RenderPassLayoutCache* renderPassLayoutCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);
    delete renderPassLayoutCache;

    // delete render pass cache
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    delete renderPassCache;

    // delete shader cache
    ShaderCache* shaderCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
    delete shaderCache;
}



//   █████╗ ██╗     ██╗      █████╗  █████╗ 
//  ██╔══██╗██║     ██║     ██╔══██╗██╔══██╗
//  ███████║██║     ██║     ██║  ██║██║  ╚═╝
//  ██╔══██║██║     ██║     ██║  ██║██║  ██╗
//  ██║  ██║███████╗███████╗╚█████╔╝╚█████╔╝
//  ╚═╝  ╚═╝╚══════╝╚══════╝ ╚════╝  ╚════╝ 

// ------------------------------------------------------------------------- //
//                 Buffer                                                    //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::allocate(Handle<Buffer> handle, VkBufferCreateInfo& info){
    Buffer* buffer = get<Buffer>(handle);
    
    // allocation info
    VmaAllocationCreateInfo allocationInfo;
    if (buffer->memType == (HOST|DEVICE)){
        // host visible + device local
        allocationInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                   | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO
        };
    } else if (buffer->memType == DEVICE){
        // device local
        uint32 dedicated = buffer->byteSize >= (1<<21) ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;
        allocationInfo = {
            .flags = dedicated,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
    } else {
        // host local
        allocationInfo = {
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        };
    }

    // create/allocate buffer
    vmaCreateBuffer(*m_allocator, &info, &allocationInfo, &buffer->buffer, &buffer->alloc, &buffer->allocInfo);

    return;
}



// ------------------------------------------------------------------------- //
//                 Texture                                                   //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::allocate(Handle<Texture> handle, VkImageCreateInfo& info){
    Texture* texture = get<Texture>(handle);

    // allocation info
    uint32 dedicated = texture->defaultRes ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;
    VmaAllocationCreateInfo allocationInfo {
        .flags = dedicated,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
    };

    // create/allocate texture
    vmaCreateImage(*m_allocator, &info, &allocationInfo, &texture->image, &texture->alloc, &texture->allocInfo);

    return;
}



//   █████╗ ██████╗ ███████╗ █████╗ ████████╗███████╗
//  ██╔══██╗██╔══██╗██╔════╝██╔══██╗╚══██╔══╝██╔════╝
//  ██║  ╚═╝██████╔╝█████╗  ███████║   ██║   █████╗  
//  ██║  ██╗██╔══██╗██╔══╝  ██╔══██║   ██║   ██╔══╝  
//  ╚█████╔╝██║  ██║███████╗██║  ██║   ██║   ███████╗
//   ╚════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝   ╚═╝   ╚══════╝

// ------------------------------------------------------------------------- //
//                 Buffer                                                    //
// ------------------------------------------------------------------------- //
template<>
Handle<Buffer> VulkanResourceManager::create<Buffer>(BufferDesc desc){
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);

    // build buffer create info
    VkBufferCreateInfo bufferInfo {
        .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size        = desc.byteSize,
        .usage       = desc.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };


    // create buffer resource
    Buffer buffer {
        .byteSize = desc.byteSize,
        //.buffer   = vulkanBuffer,
        .memType  = desc.memType
    };
    Handle<Buffer> bufferHandle = bufferCache->insert(buffer);

    // allocate and return
    allocate<Buffer>(bufferHandle, bufferInfo);
    return bufferHandle;
}



// ------------------------------------------------------------------------- //
//                 Texture                                                   //
// ------------------------------------------------------------------------- //
template<>
Handle<Texture> VulkanResourceManager::create<Texture>(TextureDesc desc){
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);

    // check if default resolution
    bool defaultRes = !desc.dimensions.x && !desc.dimensions.y && !desc.dimensions.z;
    glm::uvec3 dimensions = defaultRes ? glm::uvec3(m_screenDim.x,m_screenDim.y,m_screenDim.z)
                                       : glm::uvec3(desc.dimensions.x,desc.dimensions.y,desc.dimensions.z);
    
    // build image create info
    VkImageCreateInfo imageInfo {
        .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType   = VK_IMAGE_TYPE_2D,
        .format      = (VkFormat)desc.format,
        .extent      = {
            dimensions.x,
            dimensions.y,
            dimensions.z
        },
        .mipLevels   = desc.view.mips,
        .arrayLayers = desc.view.layers,
        .samples     = (VkSampleCountFlagBits)desc.samples,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .usage       = (VkImageUsageFlags)desc.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    // create texture resource
    Texture texture {
        // .image      = vulkanImage,
        // .view       = vulkanImageView,
        // .sampler    = vulkanSampler,
        .format     = (VkFormat)desc.format,
        .dimensions = dimensions,
        .mips       = desc.view.mips,
        .layers     = desc.view.layers,
        .samples    = (VkSampleCountFlagBits)desc.samples,
        .usage      = (VkImageUsageFlags)desc.usage,
        // .subresourceRange = subresourceRange,
        .defaultRes = defaultRes
    };
    Handle<Texture> textureHandle = textureCache->insert(texture);
    Texture* textureResource = get<Texture>(textureHandle);
    allocate<Texture>(textureHandle, imageInfo);

    // create image sampler (default)
    VkSamplerCreateInfo samplerInfo {
        .sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter     = (VkFilter)desc.sampler.magFilter,
        .minFilter     = (VkFilter)desc.sampler.minFilter,
        .mipmapMode    = (VkSamplerMipmapMode)desc.sampler.mipmapMode,
        .addressModeU  = (VkSamplerAddressMode)desc.sampler.addressing,
        .addressModeV  = (VkSamplerAddressMode)desc.sampler.addressing,
        .addressModeW  = (VkSamplerAddressMode)desc.sampler.addressing,
        .mipLodBias    = 0.f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = desc.sampler.anisotropy,
        .compareEnable = VK_TRUE,
        .compareOp     = (VkCompareOp)desc.sampler.compare,
        .minLod        = 0,
        .maxLod        = VK_LOD_CLAMP_NONE,
        .borderColor   = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    // create image view (default)
    VkImageSubresourceRange subresourceRange {
        .aspectMask     = (desc.usage == Flags::ImageUsage::IU_DEPTH_STENCIL_ATTACHMENT)
                            ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = desc.view.baseMip,
        .levelCount     = desc.view.mips,
        .baseArrayLayer = desc.view.baseLayer,
        .layerCount     = desc.view.layers
    };
    VkImageViewCreateInfo viewInfo {
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image      = textureResource->image,
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = (VkFormat)desc.format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_R,
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A
        },
        .subresourceRange = subresourceRange
    };
    textureResource->subresourceRange = subresourceRange;

    
    VK_CHECK(vkCreateSampler(m_device, &samplerInfo, NULL, &textureResource->sampler));
    VK_CHECK(vkCreateImageView(m_device, &viewInfo, NULL, &textureResource->view));
    return textureHandle;
}



// ------------------------------------------------------------------------- //
//                 Texture Attachment                                        //
// ------------------------------------------------------------------------- //
template<>
Handle<TextureAttachment> VulkanResourceManager::create<TextureAttachment>(TextureAttachmentDesc desc){
    TextureAttachmentCache* textureAttachmentCache = ((TextureAttachmentCache*) m_resourceMap[typeid(TextureAttachment)]);

    // empty Texture Attachment
    TextureAttachment textureAttachment;

    // create per-frame textures
    for (uint32 frame = 0; frame < MAX_FRAME_COUNT; frame++){
        textureAttachment.textures[frame] = create<Texture>(desc.textureLayout);
    }

    // return handle to attachment
    return textureAttachmentCache->insert(textureAttachment);
}



// ------------------------------------------------------------------------- //
//                 Descriptor Set Layout                                     //
// ------------------------------------------------------------------------- //
template<>
Handle<DescriptorSetLayout> VulkanResourceManager::create<DescriptorSetLayout>(DescriptorSetLayoutDesc desc){
    DescriptorSetLayoutCache* descriptorSetLayoutCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);

    // meta
    uint32 bindingCount = (desc.buffers.size() + desc.textures.size());

    // create descriptor set layout bindings
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    {
        // buffers
        for (auto buffer : desc.buffers){
            VkDescriptorSetLayoutBinding bufferBinding {
                .binding         = buffer.slot,
                .descriptorType  = (VkDescriptorType)buffer.type,
                .descriptorCount = 1,
                .stageFlags      = buffer.stages
            };
            bindings.push_back(bufferBinding);
        }
        // images/textures
        for (auto texture : desc.textures){
            VkDescriptorSetLayoutBinding bufferBinding {
                .binding         = texture.slot,
                .descriptorType  = (VkDescriptorType)texture.type,
                .descriptorCount = 1,
                .stageFlags      = texture.stages
            };
            bindings.push_back(bufferBinding);
        }
    }

    // build descriptor set layout info
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = bindingCount,
        .pBindings = bindings.data()
    };

    // create descriptor set layout
    VkDescriptorSetLayout vulkanDescriptorSetLayout;
    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutInfo, NULL, &vulkanDescriptorSetLayout));

    // create descriptor set layout resource, return handle
    DescriptorSetLayout descriptorSetLayout {
        .textureLayouts      = desc.textures,
        .bufferLayouts       = desc.buffers,
        .descriptorSetLayout = vulkanDescriptorSetLayout
    };
    return descriptorSetLayoutCache->insert(descriptorSetLayout);
}



// ------------------------------------------------------------------------- //
//                 Descriptor Set                                            //
// ------------------------------------------------------------------------- //
template<>
Handle<DescriptorSet> VulkanResourceManager::create<DescriptorSet>(DescriptorSetDesc desc){
    DescriptorSetCache* descriptorSetCache = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);

    // descriptor set layout from handle in desc
    DescriptorSetLayout* layout = get<DescriptorSetLayout>(desc.layout);

    // meta
    uint32 descriptorSetCount = desc.dynamic ? MAX_FRAME_COUNT : 1;

    std::vector<VkDescriptorSet> vulkanDescriptorSets;
    for (uint32 i = 0; i < descriptorSetCount; i++){
        // build descriptor set info
        VkDescriptorPool descriptorPool = desc.dynamic ? m_dynamicDescriptorPools[i] : m_globalDescriptorPool;
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = desc.dynamic ? m_dynamicDescriptorPools[i] : m_globalDescriptorPool,
            .descriptorSetCount = descriptorSetCount,
            .pSetLayouts = &(layout->descriptorSetLayout)
        };

        // allocate descriptor sets
        VkDescriptorSet vulkanDescriptorSet;
        VK_CHECK(vkAllocateDescriptorSets(m_device, NULL, &vulkanDescriptorSet));
        vulkanDescriptorSets.push_back(vulkanDescriptorSet);

        // write buffer descriptor sets
        uint32 bufferIndex = 0;
        for (auto bufferBinding : desc.buffers){
            // build buffer info
            VkDescriptorBufferInfo bufferInfo {
                .buffer = get<Buffer>(bufferBinding.buffer)->buffer,
                .offset = bufferBinding.byteOffset,
                .range  = (bufferBinding.byteSize == DescriptorSetDesc::ALL_BYTES)
                        ? VK_WHOLE_SIZE : bufferBinding.byteSize,
            };

            // build and push descriptor set write
            VkWriteDescriptorSet descriptorSetWrite {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = vulkanDescriptorSet,
                .dstBinding      = layout->bufferLayouts[bufferIndex].slot,
                .descriptorCount = 1,
                .descriptorType  = (VkDescriptorType)layout->bufferLayouts[bufferIndex].type,
                .pBufferInfo     = &bufferInfo
            };
            
            // update descriptor set
            vkUpdateDescriptorSets(m_device, 1, &descriptorSetWrite, 0, NULL);

            bufferIndex++;
        }

        // write texture descriptor sets
        uint32 textureIndex = 0;
        for (auto textureBinding : desc.textures){
            // build texture info
            VkDescriptorImageInfo textureInfo {
                .sampler      = get<Texture>(textureBinding.texture)->sampler,
                .imageView    = get<Texture>(textureBinding.texture)->view,
                .imageLayout  = VK_IMAGE_LAYOUT_PREINITIALIZED
            };

            // build and push descriptor set write
            VkWriteDescriptorSet descriptorSetWrite {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = vulkanDescriptorSet,
                .dstBinding      = layout->textureLayouts[textureIndex].slot,
                .descriptorCount = 1,
                .descriptorType  = (VkDescriptorType)layout->textureLayouts[textureIndex].type,
                .pImageInfo      = &textureInfo
            };
            
            // update descriptor set
            vkUpdateDescriptorSets(m_device, 1, &descriptorSetWrite, 0, NULL);

            textureIndex++;
        }
    }
    
    // create descriptor set resource, return handle
    DescriptorSet descriptorSet {
        .dynamic = desc.dynamic,
        .descriptorSets = vulkanDescriptorSets
    };
    return descriptorSetCache->insert(descriptorSet);
}



// ------------------------------------------------------------------------- //
//                 Render Pass Layout                                        //
// ------------------------------------------------------------------------- //
template<>
Handle<RenderPassLayout> VulkanResourceManager::create<RenderPassLayout>(RenderPassLayoutDesc desc){
    RenderPassLayoutCache* renderPassLayoutCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);

    // meta
    bool depthAttachment = desc.subpass.depthAttachment;
    uint32 attachmentCount = desc.colorAttatchmentFormats.size();

    // create attachment descriptions
    std::vector<VkAttachmentDescription> descriptions(attachmentCount);
    {
        // color 
        for (int i = 0; i < attachmentCount; i++)
            descriptions[i] = {
                .format = (VkFormat)desc.colorAttatchmentFormats[i],
            };
        // depth 
        if (depthAttachment)
            descriptions[attachmentCount] = {
                .format = (VkFormat)desc.depthAttachmentFormat
            };
    }

    // create attachment references
    VkAttachmentReference depthReference;
    std::vector<VkAttachmentReference> colorReferences;
    {
        // color
        for (uint32 colorAttachment : desc.subpass.colorAttachments)
            colorReferences.push_back({
                .attachment = colorAttachment,
                .layout = (VkImageLayout)Flags::ImageLayout::COLOR_ATTACHMENT
            });
        // depth
        if (depthAttachment)
            depthReference = {
                .attachment = (uint32)attachmentCount,
                .layout = (VkImageLayout)Flags::ImageLayout::DEPTH_STENCIL_ATTACHMENT
            };
    }

    // build subpass description
    VkSubpassDescription subpassDescription {
        .pipelineBindPoint       = (VkPipelineBindPoint)Flags::BindPoint::BP_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = NULL,
        .colorAttachmentCount    = attachmentCount,
        .pColorAttachments       = colorReferences.data(),
        .pResolveAttachments     = NULL,
        .pDepthStencilAttachment = depthAttachment ? &depthReference : NULL,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = NULL
    };

    // create render pass layout resource, return handle
    RenderPassLayout renderPassLayout {
        .depthReference         = depthReference,
        .colorReferences        = colorReferences,
        .attachmentDescriptions = descriptions,
        .subpassDescription     = subpassDescription,
        .attachmentCount        = (uint32)attachmentCount + (depthAttachment ? 1 : 0)
    };
    return renderPassLayoutCache->insert(renderPassLayout);
}



// ------------------------------------------------------------------------- //
//                 Render Pass                                               //
// ------------------------------------------------------------------------- //
template<>
Handle<RenderPass> VulkanResourceManager::create<RenderPass>(RenderPassDesc desc){
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);

    // layout data from handle in desc
    RenderPassLayout* layout = get<RenderPassLayout>(desc.layout);
    
    // insert new description info into attachment descriptions
    uint32 samples;
    bool hasDepthAttachment = (layout->colorReferences.size() + 1) == layout->attachmentCount;
    // depth
    if (hasDepthAttachment) {
        VkAttachmentDescription newAttachment = {
            .format         = layout->attachmentDescriptions[layout->attachmentCount-1].format,
            .samples        = (VkSampleCountFlagBits)desc.depthAttachment.samples,
            .loadOp         = (VkAttachmentLoadOp)desc.depthAttachment.loadOp,
            .storeOp        = (VkAttachmentStoreOp)desc.depthAttachment.storeOp,
            .stencilLoadOp  = (VkAttachmentLoadOp)desc.depthAttachment.stencilLoadOp,
            .stencilStoreOp = (VkAttachmentStoreOp)desc.depthAttachment.stencilStoreOp,
            .initialLayout  = (VkImageLayout)desc.depthAttachment.layout,
            .finalLayout    = (VkImageLayout)desc.depthAttachment.finalLayout
        };
        layout->attachmentDescriptions[layout->attachmentCount-1] = newAttachment;
        samples = desc.depthAttachment.samples;
    }
    // color
    int attachmentIndex = 0;
    for (auto attachment : desc.colorAttachments){
        VkAttachmentDescription newAttachment = {
            .format         = layout->attachmentDescriptions[attachmentIndex].format,
            .samples        = (VkSampleCountFlagBits)attachment.samples,
            .loadOp         = (VkAttachmentLoadOp)attachment.loadOp,
            .storeOp        = (VkAttachmentStoreOp)attachment.storeOp,
            .stencilLoadOp  = (VkAttachmentLoadOp)attachment.stencilLoadOp,
            .stencilStoreOp = (VkAttachmentStoreOp)attachment.stencilStoreOp,
            .initialLayout  = (VkImageLayout)attachment.layout,
            .finalLayout    = (VkImageLayout)attachment.finalLayout
        };
        layout->attachmentDescriptions[attachmentIndex] = newAttachment;
        attachmentIndex++;
        samples = attachment.samples;
    }
    
    // build render pass create info
    VkRenderPassCreateInfo createInfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = layout->attachmentCount,
        .pAttachments    = layout->attachmentDescriptions.data(),
        .subpassCount    = 1,
        .pSubpasses      = &(layout->subpassDescription),
        .dependencyCount = 0,
        .pDependencies   = NULL
    };

    // create vulkan render pass
    VkRenderPass vulkanRenderPass;
    VK_CHECK(vkCreateRenderPass(m_device, &createInfo, NULL, &vulkanRenderPass));

    // create framebuffers (one per frame)
    std::vector<VkFramebuffer> vulkanFramebuffers(MAX_FRAME_COUNT);
    for (int frame = 0; frame < MAX_FRAME_COUNT; frame++){
        // get attachment image views
        VkImageView attachments[layout->attachmentCount];
        for (uint32 i = 0; i < layout->attachmentCount-(hasDepthAttachment); i++){
            TextureAttachment* textureAttachment = get<TextureAttachment>(desc.colorAttachments[i].texture);
            Texture* texture =  get<Texture>(textureAttachment->textures[frame]);
            attachments[i] = texture->view;
        }
        if(hasDepthAttachment){
            TextureAttachment* textureAttachment = get<TextureAttachment>(desc.depthAttachment.texture);
            Texture* texture =  get<Texture>(textureAttachment->textures[frame]);
            attachments[layout->attachmentCount-1] = texture->view;
        }

        // build framebuffer create info
        VkFramebufferCreateInfo framebufferInfo {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = vulkanRenderPass,
            .attachmentCount = layout->attachmentCount,
            .pAttachments    = attachments,
            .width           = desc.dimensions.x,
            .height          = desc.dimensions.y,
            .layers          = desc.dimensions.z
        };

        // create vulkan framebuffer
        VkFramebuffer vulkanFramebuffer;
        VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, NULL, &vulkanFramebuffer));

        vulkanFramebuffers[frame] = vulkanFramebuffer;
    }
    

    // create render pass resource, return handle
    RenderPass renderPass {
        .layout             = desc.layout,
        .renderPass         = vulkanRenderPass,
        .framebuffers       = vulkanFramebuffers,
        .dimensions         = desc.dimensions,
        .samples            = samples,
        .hasDepthAttachment = hasDepthAttachment,
        .depthAttachment    = desc.depthAttachment,
        .colorAttachments   = desc.colorAttachments
    };
    return renderPassCache->insert(renderPass);
}



// ------------------------------------------------------------------------- //
//                 Shader                                                    //
// ------------------------------------------------------------------------- //
template<>
Handle<Shader> VulkanResourceManager::create<Shader>(ShaderDesc desc){
    ShaderCache* shaderCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);

    // meta
    RenderPass* renderPass = get<RenderPass>(desc.graphicsState.renderPass);
    RenderPassLayout* renderPassLayout = get<RenderPassLayout>(renderPass->layout);

    // create vertex shader module
    VkShaderModule vertexShader;
    bool hasVertexShader = false;
    if (desc.vertexShader.shaderPath.compare("") == 0){
        hasVertexShader = true;

        // get shader bytes
        std::ifstream instream(desc.vertexShader.shaderPath, std::ios::in | std::ios::binary);
        std::vector<uint8> bytes = std::vector<uint8>((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
        uint32 size = bytes.size();

        // build shader module info
        VkShaderModuleCreateInfo shaderModuleInfo {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size,
            .pCode    = reinterpret_cast<const uint32_t*>(bytes.data())
        };

        // create shader module
        VK_CHECK(vkCreateShaderModule(m_device, &shaderModuleInfo, NULL, &vertexShader));
    }

    // create fragment shader module
    VkShaderModule fragmentShader;
    bool hasFragmentShader = false;
    if (desc.fragmentShader.shaderPath.compare("") == 0){
        hasFragmentShader = true;

        // get shader bytes
        std::ifstream instream(desc.fragmentShader.shaderPath, std::ios::in | std::ios::binary);
        std::vector<uint8> bytes = std::vector<uint8>((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
        uint32 size = bytes.size();

        // build shader module info
        VkShaderModuleCreateInfo shaderModuleInfo {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = size,
            .pCode    = reinterpret_cast<const uint32_t*>(bytes.data())
        };

        // create shader module
        VK_CHECK(vkCreateShaderModule(m_device, &shaderModuleInfo, NULL, &fragmentShader));
    }

    // create shader stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    if (hasVertexShader) // vertex
        shaderStages.push_back({
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName  = "main"
        });
    if (fragmentShader) // fragment
        shaderStages.push_back({
            .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName  = "main"
        });

    // build pipeline layout info
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    for (auto setLayoutHandle : desc.descriptorSets) {
        DescriptorSetLayout* setLayout = get<DescriptorSetLayout>(setLayoutHandle);
        descriptorSetLayouts.push_back(setLayout->descriptorSetLayout);
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount         = (uint32)descriptorSetLayouts.size(),
        .pSetLayouts            = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = NULL
    };

    // create pipeline layout
    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, NULL, &pipelineLayout));

    // graphics state meta
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(renderPassLayout->attachmentCount);
    std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    // build graphics state info
    VkPipelineVertexInputStateCreateInfo   vertexInputState;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
    VkPipelineViewportStateCreateInfo      viewportState;
    VkPipelineRasterizationStateCreateInfo rasterizationState;
    VkPipelineMultisampleStateCreateInfo   multisampleState;
    VkPipelineDepthStencilStateCreateInfo  depthStencilState;
    VkPipelineColorBlendStateCreateInfo    colorBlendState;
    VkPipelineDynamicStateCreateInfo       dynamicState;
    {
        // build vertex input state info
        vertexInputState = {
            .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount   = 0,
            .pVertexBindingDescriptions      = NULL,
            .vertexAttributeDescriptionCount = 0,
            .pVertexAttributeDescriptions    = NULL
        };

        // build input assembly state info
        inputAssemblyState = {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE
        };

        // build viewport state info
        viewportState = {
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount  = 1
        };

        // build rasterization state info
        rasterizationState = {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = VK_POLYGON_MODE_FILL,
            .cullMode                = VK_CULL_MODE_BACK_BIT,
            .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .depthBiasEnable         = VK_FALSE
        };

        // build multisample state info
        RenderPass* renderPass = get<RenderPass>(desc.graphicsState.renderPass);
        RenderPassLayout* renderPassLayout = get<RenderPassLayout>(renderPass->layout);
        uint32 sampleCount = renderPass->samples;
        multisampleState = {
            .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples  = (VkSampleCountFlagBits)sampleCount,
            .sampleShadingEnable   = VK_FALSE,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable      = VK_FALSE
        };  

        // build depth stencil state info
        depthStencilState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable       = VK_TRUE,
            .depthWriteEnable      = VK_TRUE,
            .depthCompareOp        = (VkCompareOp) desc.graphicsState.depthTest,
            .depthBoundsTestEnable = VK_FALSE,
            .stencilTestEnable     = VK_FALSE
        };

        // build color blend attachments
        for (int i = 0; i < renderPassLayout->attachmentCount; i++){
            colorBlendAttachments[i] = {
                .blendEnable         = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp        = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp        = VK_BLEND_OP_ADD,
                .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
                                  VK_COLOR_COMPONENT_G_BIT | 
                                  VK_COLOR_COMPONENT_B_BIT | 
                                  VK_COLOR_COMPONENT_A_BIT
            };
        }

        // build color blend state
        colorBlendState = {
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable   = VK_FALSE,
            .attachmentCount = renderPassLayout->attachmentCount,
            .pAttachments    = colorBlendAttachments.data(),
            .blendConstants  = {
                1.f,1.f,1.f,1.f
            }
        };

        // build dynamic state
        dynamicState = {
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = 2,
            .pDynamicStates    = dynamicStates.data()
        };
    }

    // build pipeline info
    VkGraphicsPipelineCreateInfo pipelineInfo {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount          = (uint32)(hasVertexShader + hasFragmentShader),
        .pVertexInputState   = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pTessellationState  = NULL,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState   = &multisampleState,
        .pDepthStencilState  = &depthStencilState,
        .pColorBlendState    = &colorBlendState,
        .layout              = pipelineLayout,
        .renderPass          = renderPass->renderPass,
        .subpass             = 0
    };

    // create pipeline
    VkPipeline vulkanPipeline;
    VK_CHECK(vkCreateGraphicsPipelines(m_device, NULL, 1, &pipelineInfo, NULL, &vulkanPipeline));

    // create shader resource, return handle
    Shader shader {
        .pipeline = vulkanPipeline
    };
    return shaderCache->insert(shader);
}



//  ██████╗ ███████╗ █████╗ ██████╗ ███████╗ █████╗ ████████╗███████╗
//  ██╔══██╗██╔════╝██╔══██╗██╔══██╗██╔════╝██╔══██╗╚══██╔══╝██╔════╝
//  ██████╔╝█████╗  ██║  ╚═╝██████╔╝█████╗  ███████║   ██║   █████╗  
//  ██╔══██╗██╔══╝  ██║  ██╗██╔══██╗██╔══╝  ██╔══██║   ██║   ██╔══╝  
//  ██║  ██║███████╗╚█████╔╝██║  ██║███████╗██║  ██║   ██║   ███████╗
//  ╚═╝  ╚═╝╚══════╝ ╚════╝ ╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝   ╚═╝   ╚══════╝

// ------------------------------------------------------------------------- //
//                 RenderPass                                                //
// ------------------------------------------------------------------------- //
template<>
Handle<RenderPass> VulkanResourceManager::recreate<RenderPass>(Handle<RenderPass> handle, glm::uvec2 newDimensions){ // TODO: realloc textures
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    RenderPass* renderPass = get<RenderPass>(handle);

    // update screen dimensions
    m_screenDim = glm::uvec3(newDimensions.x, newDimensions.y, m_screenDim.z);

    // attachment info
    bool hasDepthAttachment = renderPass->depthAttachment.texture.isValid();
    uint32 attachmentCount = hasDepthAttachment + renderPass->colorAttachments.size();
    std::vector<RenderPass::ColorAttachment> colorAttachments = renderPass->colorAttachments;
    RenderPass::DepthAttachment depthAttachment = renderPass->depthAttachment;
    
    // destroy framebuffers
    for (int i = 0; i < MAX_FRAME_COUNT; i++){
        vkDestroyFramebuffer(m_device, renderPass->framebuffers[i], NULL);
    }

    // rebuild all textures that use default res
    {   // color
        for (RenderPass::ColorAttachment& attachment : colorAttachments){
            // get color attachment
            TextureAttachment* textureAttachment = get<TextureAttachment>(attachment.texture);

            // recreate attachment's images/views
            for(int frame = 0; frame < MAX_FRAME_COUNT; frame++){
                // get texture
                Handle<Texture> textureHandle = textureAttachment->textures[frame];
                Texture* texture = get<Texture>(textureHandle);

                // only need to create those that match screen res
                if(!texture->defaultRes)
                    continue;
                

                // destroy image view
                vkDestroyImageView(m_device, texture->view, NULL);

                // destroy image
                vmaDestroyImage(*m_allocator, texture->image, texture->alloc);

                // create image
                VkImageCreateInfo imageInfo {
                    .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .imageType   = VK_IMAGE_TYPE_2D,
                    .format      = texture->format,
                    .extent      = {
                        newDimensions.x,
                        newDimensions.y,
                        1
                    },
                    .mipLevels   = texture->mips,
                    .arrayLayers = texture->layers,
                    .samples     = texture->samples,
                    .tiling      = VK_IMAGE_TILING_OPTIMAL,
                    .usage       = texture->usage,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
                };
                allocate<Texture>(textureHandle, imageInfo);      

                // create image view
                VkImageViewCreateInfo viewInfo {
                    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image      = texture->image,
                    .viewType   = VK_IMAGE_VIEW_TYPE_2D,
                    .format     = texture->format,
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A
                    },
                    .subresourceRange = texture->subresourceRange
                };
                
                // update texture object
                VK_CHECK(vkCreateImageView(m_device, &viewInfo, NULL, &texture->view));
                texture->dimensions = glm::uvec3(newDimensions.x,newDimensions.y,1);
            }
            
        }
    }
    {   // depth
        if (hasDepthAttachment){
            // get color attachment
            TextureAttachment* textureAttachment = get<TextureAttachment>(depthAttachment.texture);

            // recreate attachment's images/views
            for(int frame = 0; frame < MAX_FRAME_COUNT; frame++){
                // get texture
                Handle<Texture> textureHandle = textureAttachment->textures[frame];
                Texture* texture = get<Texture>(textureHandle);

                // only need to create those that match screen res
                if(!texture->defaultRes)
                    continue;
                

                // destroy image view
                vkDestroyImageView(m_device, texture->view, NULL);

                // destroy image
                vmaDestroyImage(*m_allocator, texture->image, texture->alloc);

                // create image
                VkImageCreateInfo imageInfo {
                    .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .imageType   = VK_IMAGE_TYPE_2D,
                    .format      = texture->format,
                    .extent      = {
                        newDimensions.x,
                        newDimensions.y,
                        1
                    },
                    .mipLevels   = texture->mips,
                    .arrayLayers = texture->layers,
                    .samples     = texture->samples,
                    .tiling      = VK_IMAGE_TILING_OPTIMAL,
                    .usage       = texture->usage,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE
                };
                allocate<Texture>(textureHandle, imageInfo);      

                // create image view
                VkImageViewCreateInfo viewInfo {
                    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image      = texture->image,
                    .viewType   = VK_IMAGE_VIEW_TYPE_2D,
                    .format     = texture->format,
                    .components = {
                        .r = VK_COMPONENT_SWIZZLE_R,
                        .g = VK_COMPONENT_SWIZZLE_G,
                        .b = VK_COMPONENT_SWIZZLE_B,
                        .a = VK_COMPONENT_SWIZZLE_A
                    },
                    .subresourceRange = texture->subresourceRange
                };
                
                // update texture object
                VK_CHECK(vkCreateImageView(m_device, &viewInfo, NULL, &texture->view));
                texture->dimensions = glm::uvec3(newDimensions.x,newDimensions.y,1);
            }
        }
    }
        
    // rebuild framebuffers
    std::vector<VkFramebuffer>& vulkanFramebuffers = renderPass->framebuffers;
    VkImageView vulkanAttachments[MAX_FRAME_COUNT][attachmentCount];
    
    // get attachment image views
    for (uint32 i = 0; i < attachmentCount+hasDepthAttachment; i++){
        TextureAttachment* textureAttachment = get<TextureAttachment>(colorAttachments[i].texture);
        for (int frame = 0;frame < MAX_FRAME_COUNT; frame++){
            Texture* texture = get<Texture>(textureAttachment->textures[frame]);
            vulkanAttachments[frame][i] = texture->view;
        }
    }
    if (hasDepthAttachment){
        TextureAttachment* textureAttachment = get<TextureAttachment>(depthAttachment.texture);
        for (int frame = 0; frame < MAX_FRAME_COUNT; frame++){
            Texture* texture = get<Texture>(textureAttachment->textures[frame]);
            vulkanAttachments[frame][attachmentCount-1] = texture->view;
        }
    }

    for (int frame = 0; frame < MAX_FRAME_COUNT; frame++){
        // build framebuffer create info
        VkFramebufferCreateInfo framebufferInfo {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = renderPass->renderPass,
            .attachmentCount = attachmentCount,
            .pAttachments    = vulkanAttachments[frame],
            .width           = newDimensions.x,
            .height          = newDimensions.y,
            .layers          = 1
        };

        // create vulkan framebuffer
        VkFramebuffer vulkanFramebuffer;
        VK_CHECK(vkCreateFramebuffer(m_device, &framebufferInfo, NULL, &vulkanFramebuffers[frame]));
    }
    
    return handle;
}



//  ██████╗ ███████╗███╗   ███╗ █████╗ ██╗   ██╗███████╗
//  ██╔══██╗██╔════╝████╗ ████║██╔══██╗██║   ██║██╔════╝
//  ██████╔╝█████╗  ██╔████╔██║██║  ██║╚██╗ ██╔╝█████╗  
//  ██╔══██╗██╔══╝  ██║╚██╔╝██║██║  ██║ ╚████╔╝ ██╔══╝  
//  ██║  ██║███████╗██║ ╚═╝ ██║╚█████╔╝  ╚██╔╝  ███████╗
//  ╚═╝  ╚═╝╚══════╝╚═╝     ╚═╝ ╚════╝    ╚═╝   ╚══════╝

// ------------------------------------------------------------------------- //
//                 Buffer                                                    //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<Buffer>(Handle<Buffer> handle){
    BufferCache* resourceCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
};


// ------------------------------------------------------------------------- //
//                 Texture                                                   //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<Texture>(Handle<Texture> handle){
    TextureCache* resourceCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
};


// ------------------------------------------------------------------------- //
//                 DescriptorSetLayout                                       //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<DescriptorSetLayout>(Handle<DescriptorSetLayout> handle){
    DescriptorSetLayoutCache* resourceCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
};


// ------------------------------------------------------------------------- //
//                 DescriptorSet                                             //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<DescriptorSet>(Handle<DescriptorSet> handle){
    DescriptorSetCache* resourceCache = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
};


// ------------------------------------------------------------------------- //
//                 RenderPassLayout                                          //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<RenderPassLayout>(Handle<RenderPassLayout> handle){
    RenderPassLayoutCache* resourceCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);
};


// ------------------------------------------------------------------------- //
//                 RenderPass                                                //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<RenderPass>(Handle<RenderPass> handle){
    RenderPassCache* resourceCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
};


// ------------------------------------------------------------------------- //
//                 Shader                                                    //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<Shader>(Handle<Shader> handle){
    ShaderCache* resourceCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
};

}