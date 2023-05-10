#include "VulkanResourceManager.h"
#include <vulkan/vulkan_core.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

#include "ResourceFlags.h"
#include "ResourceTypes.h"
#include <filesystem>
#include "../../external/volk/volk.h"
#include <fstream>
#include "memory/Pool.h"
#include "util/FunctionQueue.h"
#include "../VulkanDevice.h"
#include "../debug/SprLog.h"


namespace spr::gfx {


//  ██╗███╗  ██╗██╗████████╗
//  ██║████╗ ██║██║╚══██╔══╝
//  ██║██╔██╗██║██║   ██║   
//  ██║██║╚████║██║   ██║   
//  ██║██║ ╚███║██║   ██║   
//  ╚═╝╚═╝  ╚══╝╚═╝   ╚═╝   

VulkanResourceManager::VulkanResourceManager(){}
VulkanResourceManager::~VulkanResourceManager(){
    if (m_destroyed)
        return;
    
    SprLog::warn("[VulkanResourceManager] [~] Calling destroy() in destructor");
    destroy();
}

void VulkanResourceManager::init(VulkanDevice& device, glm::uvec3 screenDim){
    m_screenDim = screenDim;
    m_device = device.getDevice();
    
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &properties);
    m_minUBOAlignment = properties.limits.minUniformBufferOffsetAlignment;
    
    // create allocator
    VmaVulkanFunctions pFunctions = {
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr
    };
    VmaAllocatorCreateInfo allocatorCreateInfo = {
        .physicalDevice   = device.getPhysicalDevice(),
        .device           = device.getDevice(),
        .pVulkanFunctions = &pFunctions,
        .instance         = device.getInstance(),
        .vulkanApiVersion = VK_MAKE_API_VERSION(0, 1, 2, 170),
    };
    VK_CHECK(vmaCreateAllocator(&allocatorCreateInfo, &m_allocator));

    // setup global descriptor pool
    std::vector<VkDescriptorPoolSize> globalPoolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            4096 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,            4096 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,            32768 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   32768 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,             4096 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,          4096 },
        { VK_DESCRIPTOR_TYPE_SAMPLER,                  32768 }
    };
    VkDescriptorPoolCreateInfo globalPoolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .maxSets = 4096,
        .poolSizeCount = (uint32)globalPoolSizes.size(),
        .pPoolSizes = globalPoolSizes.data()
    };
    VK_CHECK(vkCreateDescriptorPool(m_device, &globalPoolInfo, NULL, &m_globalDescriptorPool));

    // setup dynamic descriptor pools
    for (int frame = 0; frame < MAX_FRAME_COUNT; frame++){
        std::vector<VkDescriptorPoolSize> dynamicPoolSizes = {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            4096 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,            4096 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,            32768 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   32768 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,             4096 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,          4096 },
            { VK_DESCRIPTOR_TYPE_SAMPLER,                  32768 }
        };
        VkDescriptorPoolCreateInfo dynamicPoolInfo = {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .pNext = NULL,
            .flags = 0,
            .maxSets = 4096,
            .poolSizeCount = (uint32)dynamicPoolSizes.size(),
            .pPoolSizes = dynamicPoolSizes.data()
        };
        VK_CHECK(vkCreateDescriptorPool(m_device, &dynamicPoolInfo, NULL, &m_dynamicDescriptorPools[frame]));
    }

    // setup deletion queues, 1 per frame
    for (int frame = 0; frame < MAX_FRAME_COUNT; frame++){
        m_deletionQueue[frame] = FunctionQueue();
    }
}

void VulkanResourceManager::destroy(){
    // flush deletion queue
    for(uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        m_deletionQueue[i].execute();
    }

    // delete buffer cache
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
    delete bufferCache;

    // delete texture cache
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    delete textureCache;

    // delete descriptor set cache
    DescriptorSetCache* descriptorSetCache = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
    delete descriptorSetCache;

    // delete descriptor set layout cache
    DescriptorSetLayoutCache* descriptorSetLayoutCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
    delete descriptorSetLayoutCache;

    // delete render pass layout cache
    RenderPassLayoutCache* renderPassLayoutCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);
    delete renderPassLayoutCache;

    // delete render pass cache
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    delete renderPassCache;

    // delete shader cache
    ShaderCache* shaderCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
    delete shaderCache;

    // delete texture attachment cache
    TextureAttachmentCache* textureAttachmentCache = ((TextureAttachmentCache*) m_resourceMap[typeid(TextureAttachment)]);
    delete textureAttachmentCache;

    // destroy descriptor pools
    vkDestroyDescriptorPool(m_device, m_globalDescriptorPool, nullptr);
    for(uint32 i = 0; i < MAX_FRAME_COUNT; i++){
        vkDestroyDescriptorPool(m_device, m_dynamicDescriptorPools[i], nullptr);
    }

    // destroy allocator
    vmaDestroyAllocator(m_allocator);

    m_destroyed = true;
    SprLog::info("[VulkanResourceManager] [destroy] destroyed...");
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
    uint32 testVal = buffer->byteSize;

    // allocation info
    VmaAllocationCreateInfo allocationInfo;
    if (buffer->memType == (HOST|DEVICE)){
        // host visible + device local
        allocationInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                   | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
    } else if (buffer->memType == DEVICE){
        // device local
        uint32 dedicated = buffer->byteSize >= (1<<22) ? VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT : 0;
        allocationInfo = {
            .flags = dedicated,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE
        };
    } else {
        // host local
        allocationInfo = {
            .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                   | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST
        };
    }
    
    // create/allocate buffer
    VK_CHECK(vmaCreateBuffer(m_allocator, &info, &allocationInfo, &(buffer->buffer), &(buffer->alloc), &(buffer->allocInfo)));

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
    VK_CHECK(vmaCreateImage(m_allocator, &info, &allocationInfo, &texture->image, &texture->alloc, &texture->allocInfo));
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
        .pNext       = NULL,
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
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
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
    allocate<Texture>(textureHandle, imageInfo);
    Texture* textureResource = get<Texture>(textureHandle);

    // create image sampler (default)
    VkSamplerCreateInfo samplerInfo {
        .sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext         = NULL,
        .flags         = 0,
        .magFilter     = (VkFilter)desc.sampler.magFilter,
        .minFilter     = (VkFilter)desc.sampler.minFilter,
        .mipmapMode    = (VkSamplerMipmapMode)desc.sampler.mipmapMode,
        .addressModeU  = (VkSamplerAddressMode)desc.sampler.addressing,
        .addressModeV  = (VkSamplerAddressMode)desc.sampler.addressing,
        .addressModeW  = (VkSamplerAddressMode)desc.sampler.addressing,
        .mipLodBias    = 0.f,
        .anisotropyEnable = VK_FALSE,
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
        .aspectMask     = (desc.usage & Flags::ImageUsage::IU_DEPTH_STENCIL_ATTACHMENT)
                            ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = desc.view.baseMip,
        .levelCount     = desc.view.mips,
        .baseArrayLayer = desc.view.baseLayer,
        .layerCount     = desc.view.layers
    };
    VkImageViewCreateInfo viewInfo {
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext      = NULL,
        .flags      = 0,
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
        textureAttachment.textures.push_back(create<Texture>(desc.textureLayout));
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
        for (auto& buffer : desc.buffers){
            VkDescriptorSetLayoutBinding bufferBinding {
                .binding         = buffer.binding,
                .descriptorType  = (VkDescriptorType)buffer.type,
                .descriptorCount = 1,
                .stageFlags      = buffer.stages
            };
            bindings.push_back(bufferBinding);
        }
        // images/textures
        for (auto& texture : desc.textures){
            VkDescriptorSetLayoutBinding textureBinding {
                .binding         = texture.binding,
                .descriptorType  = (VkDescriptorType)texture.type,
                .descriptorCount = texture.count,
                .stageFlags      = texture.stages
            };
            bindings.push_back(textureBinding);
        }
    }

    // build descriptor set layout info
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .bindingCount = bindingCount,
        .pBindings = bindingCount > 0 ? bindings.data() : NULL
    };

    // create descriptor set layout
    DescriptorSetLayout descriptorSetLayout {
        .textureLayouts      = desc.textures,
        .bufferLayouts       = desc.buffers,
        .descriptorSetLayout = VK_NULL_HANDLE
    };
    VK_CHECK(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutInfo, NULL, &descriptorSetLayout.descriptorSetLayout));

    // create descriptor set layout resource, return handle
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
    // get descriptor counts
    uint32 globalDescriptorCount = 0;
    uint32 perFrameDescriptorCount = 0;
    for (auto binding : desc.textures){
        bool texBinding        =  binding.texture.isValid();
        bool arrayTexBinding   = !binding.textures.empty();
        bool attachmentBinding =  binding.attachment.isValid();

        if (texBinding && arrayTexBinding){
            SprLog::warn("[VulkanResourceManager] [create<DescriptorSet>] Texture binding overdefined, defaulting to single texture");
            arrayTexBinding = false;
        }

        globalDescriptorCount   += texBinding;
        globalDescriptorCount   += arrayTexBinding;
        perFrameDescriptorCount += attachmentBinding;
    }
    for (auto binding : desc.buffers){
        globalDescriptorCount   +=  binding.buffer.isValid();
        perFrameDescriptorCount += !binding.buffers.empty();
    }
    
    // validate
    if (globalDescriptorCount > 0 && perFrameDescriptorCount > 0){
        SprLog::error("[VulkanResourceManager] [create<DescriptorSet>] Cannot use both global and per-frame descriptors in one set");
    }
    if (globalDescriptorCount == 0 && perFrameDescriptorCount == 0){
        SprLog::error("[VulkanResourceManager] [create<DescriptorSet>] Cannot manually create descriptor set with no descriptors");
    }
    bool globalDescriptors = globalDescriptorCount > 0;

    // allocate and write descriptor set(s)
    //      will run once if using global descriptors (shared over frames)
    //      will run MAX_FRAME_COUNT if using per-frame descriptors (frame buffered sets)
    std::vector<VkDescriptorSet> vulkanDescriptorSets;
    uint32 descriptorSetCount = globalDescriptors ? 1 : MAX_FRAME_COUNT;

    for (uint32 frame = 0; frame < descriptorSetCount; frame++){
        // build descriptor set info
        VkDescriptorPool descriptorPool = globalDescriptors ? m_globalDescriptorPool 
                                                            : m_dynamicDescriptorPools[frame];
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext = NULL,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &(layout->descriptorSetLayout)
        };
        // allocate descriptor set
        VkDescriptorSet vulkanDescriptorSet{};
        VK_CHECK(vkAllocateDescriptorSets(m_device, &descriptorSetAllocInfo, &vulkanDescriptorSet));
        vulkanDescriptorSets.push_back(vulkanDescriptorSet);

        // write buffer descriptors 
        uint32 bufferDescriptorCount = desc.buffers.size();
        for (uint32 bufferIndex = 0; bufferIndex < bufferDescriptorCount; bufferIndex++){
            DescriptorSetDesc::BufferBinding& binding = desc.buffers[bufferIndex];
            Handle<Buffer> handle = globalDescriptors ? binding.buffer
                                                      : binding.buffers[frame];
            Buffer* buffer = get<Buffer>(handle);
            VkBuffer buffResource = buffer->buffer;

            VkDescriptorBufferInfo bufferInfo {
                .buffer = buffResource,
                .offset = (VkDeviceSize)binding.byteOffset,
                .range  = (VkDeviceSize)(binding.byteSize == DescriptorSetDesc::ALL_BYTES)
                        ? VK_WHOLE_SIZE : binding.byteSize,
            };

            VkWriteDescriptorSet descriptorSetWrite[1];
            descriptorSetWrite[0] = {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext           = NULL,
                .dstSet          = vulkanDescriptorSets.at(frame),
                .dstBinding      = layout->bufferLayouts[bufferIndex].binding,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType  = (VkDescriptorType)layout->bufferLayouts[bufferIndex].type,
                .pBufferInfo     = &bufferInfo
            };
            vkUpdateDescriptorSets(m_device, 1, descriptorSetWrite, 0, NULL);
        }

        // write texture descriptors
        uint32 textureDescriptorCount = desc.textures.size();
        for (uint32 textureIndex = 0; textureIndex < textureDescriptorCount; textureIndex++){
            DescriptorSetDesc::TextureBinding binding = desc.textures[textureIndex];

            bool usesTexture = binding.texture.isValid();
            bool usesAttachment = binding.attachment.isValid();
            bool usesTextureArray = !binding.textures.empty();

            std::vector<VkDescriptorImageInfo> textureInfos;
            if (usesAttachment) {
                Handle<TextureAttachment> attachmentHandle = binding.attachment;
                TextureAttachment* attachment = get<TextureAttachment>(attachmentHandle);
                Handle<Texture> handle = attachment->textures[frame];

                Texture* texture = get<Texture>(handle);
                VkDescriptorImageInfo textureInfo {
                    .sampler      = texture->sampler,
                    .imageView    = texture->view,
                    .imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
                textureInfos.push_back(textureInfo);

            } else if (usesTexture){
                Handle<Texture> handle = binding.texture;
                Texture* texture = get<Texture>(handle);

                VkDescriptorImageInfo textureInfo {
                    .sampler      = texture->sampler,
                    .imageView    = texture->view,
                    .imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                };
                textureInfos.push_back(textureInfo);

            } else { // usesTextureArray
                for (Handle<Texture> handle : binding.textures){
                    Texture* texture = get<Texture>(handle);

                    VkDescriptorImageInfo textureInfo {
                        .sampler      = texture->sampler,
                        .imageView    = texture->view,
                        .imageLayout  = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                    };
                    textureInfos.push_back(textureInfo);
                }
            }

            VkWriteDescriptorSet descriptorSetWrite {
                .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet          = vulkanDescriptorSets.at(frame),
                .dstBinding      = layout->textureLayouts[textureIndex].binding,
                .descriptorCount = (uint32)textureInfos.size(),
                .descriptorType  = (VkDescriptorType)layout->textureLayouts[textureIndex].type,
                .pImageInfo      = textureInfos.data()
            };
            
            vkUpdateDescriptorSets(m_device, 1, &descriptorSetWrite, 0, NULL);
        }
    }
    
    // create descriptor set resource, return handle
    DescriptorSet descriptorSet {
        .descriptorSets = vulkanDescriptorSets,
        .global = globalDescriptors
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
    std::vector<VkAttachmentDescription> descriptions(attachmentCount + depthAttachment);
    {
        // color 
        for (int i = 0; i < attachmentCount; i++){
            descriptions[i] = {
                .format = (VkFormat)desc.colorAttatchmentFormats[i],
            };
        }
        // depth 
        if (depthAttachment){
            descriptions[attachmentCount] = {
                .format = (VkFormat)desc.depthAttachmentFormat
            };
        }
    }

    // create attachment references
    VkAttachmentReference depthReference;
    std::vector<VkAttachmentReference> colorReferences;
    {
        // color
        for (uint32 colorAttachment : desc.subpass.colorAttachments){
            colorReferences.push_back({
                .attachment = colorAttachment,
                .layout = (VkImageLayout)Flags::ImageLayout::COLOR_ATTACHMENT
            });
        }
        // depth
        if (depthAttachment){
            depthReference = {
                .attachment = (uint32)attachmentCount,
                .layout = (VkImageLayout)Flags::ImageLayout::DEPTH_STENCIL_ATTACHMENT
            };
        }
    }

    // create render pass layout resource, return handle
    RenderPassLayout renderPassLayout {
        .depthReference         = depthReference,
        .colorReferences        = colorReferences,
        .attachmentDescriptions = descriptions,
        .attachmentCount        = (uint32)attachmentCount + (depthAttachment)
    };
    return renderPassLayoutCache->insert(renderPassLayout);
}



// ------------------------------------------------------------------------- //
//                 Render Pass                                               //
// ------------------------------------------------------------------------- //
template<>
Handle<RenderPass> VulkanResourceManager::create<RenderPass>(RenderPassDesc desc){
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    // meta
    RenderPassLayout* layout = get<RenderPassLayout>(desc.layout);
    uint32 attachmentCount = layout->attachmentCount;
    uint32 swapchainImageCount = desc.colorAttachments[0].swapchainImageViews.size();
    bool swapchainOverride = swapchainImageCount > 0;
    uint32 adjustedFrameCount = swapchainOverride ? swapchainImageCount : MAX_FRAME_COUNT;
    bool hasDepthAttachment = (layout->colorReferences.size() + 1) == attachmentCount;


    // check that renderpass uses exclusively attachments or swapchain images
    if (swapchainOverride && (attachmentCount > 1 || hasDepthAttachment)){
        SprLog::error("[VulkanResourceManager] [create<RenderPass>] Cannot use both swapchain images and attachments");
    }
    
    // insert new description info into attachment descriptions
    uint32 samples;    
    // depth
    if (hasDepthAttachment) {
        VkAttachmentDescription newAttachment = {
            .format         = layout->attachmentDescriptions[attachmentCount-1].format,
            .samples        = (VkSampleCountFlagBits)desc.depthAttachment.samples,
            .loadOp         = (VkAttachmentLoadOp)desc.depthAttachment.loadOp,
            .storeOp        = (VkAttachmentStoreOp)desc.depthAttachment.storeOp,
            .stencilLoadOp  = (VkAttachmentLoadOp)desc.depthAttachment.stencilLoadOp,
            .stencilStoreOp = (VkAttachmentStoreOp)desc.depthAttachment.stencilStoreOp,
            .initialLayout  = (VkImageLayout)desc.depthAttachment.layout,
            .finalLayout    = (VkImageLayout)desc.depthAttachment.finalLayout
        };
        layout->attachmentDescriptions[attachmentCount-1] = newAttachment;
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

    // build subpass dependencies
    VkSubpassDependency inDependency;
    VkSubpassDependency outDependency;
    if (swapchainOverride){ // special case for swapchain image attachments
        inDependency = {
            .srcSubpass      = VK_SUBPASS_EXTERNAL,
            .dstSubpass      = 0,
            .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask   = VK_ACCESS_NONE_KHR,
            .dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dependencyFlags = 0
        };
        outDependency = {
            .srcSubpass      = 0,
            .dstSubpass      = VK_SUBPASS_EXTERNAL,
            .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask   = VK_ACCESS_NONE_KHR,
            .dependencyFlags = 0
        };
    } else { // offscreen attachment dependencies
        inDependency = {
            .srcSubpass      = VK_SUBPASS_EXTERNAL,
            .dstSubpass      = 0,
            .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
            .dependencyFlags = 0
        };
        outDependency = {
            .srcSubpass      = 0,
            .dstSubpass      = VK_SUBPASS_EXTERNAL,
            .srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask    = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            .srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .dstAccessMask   = VK_ACCESS_SHADER_READ_BIT,
            .dependencyFlags = 0
        };
    }
    
    VkSubpassDependency dependencies[2] = {
        inDependency,
        outDependency
    };

    // build subpass description
    int32 colorAttachmentCount = hasDepthAttachment ? attachmentCount - 1 : attachmentCount;
    colorAttachmentCount = colorAttachmentCount > 0 ? colorAttachmentCount : 0;
    VkSubpassDescription subpassDescription = {
        .pipelineBindPoint       = (VkPipelineBindPoint)Flags::BindPoint::BP_GRAPHICS,
        .inputAttachmentCount    = 0,
        .pInputAttachments       = NULL,
        .colorAttachmentCount    = (uint32)colorAttachmentCount,
        .pColorAttachments       = colorAttachmentCount > 0 ? layout->colorReferences.data() : NULL,
        .pResolveAttachments     = NULL,
        .pDepthStencilAttachment = hasDepthAttachment ? &(layout->depthReference) : NULL,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments    = NULL
    };
    
    // build render pass create info
    VkRenderPassCreateInfo createInfo {
        .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext           = NULL,
        .flags           = 0,
        .attachmentCount = attachmentCount,
        .pAttachments    = layout->attachmentDescriptions.data(),
        .subpassCount    = 1,
        .pSubpasses      = &subpassDescription,
        .dependencyCount = 2,
        .pDependencies   = dependencies
    };
    // create vulkan render pass
    VkRenderPass vulkanRenderPass;
    VK_CHECK(vkCreateRenderPass(m_device, &createInfo, NULL, &vulkanRenderPass));

    // create framebuffers (one per frame) 
    std::vector<VkFramebuffer> vulkanFramebuffers(adjustedFrameCount);
    for (int frame = 0; frame < adjustedFrameCount; frame++){
        // get attachment image views
        VkImageView attachments[attachmentCount];

        // color
        for (uint32 i = 0; i < attachmentCount-(hasDepthAttachment); i++){
            // use swapchain image view directly
            if (swapchainOverride){
                attachments[i] = desc.colorAttachments[i].swapchainImageViews[frame];
                break;
            }
            
            // fetch provided attachments
            TextureAttachment* textureAttachment = get<TextureAttachment>(desc.colorAttachments[i].texture);
            Texture* texture =  get<Texture>(textureAttachment->textures[frame]);
            attachments[i] = texture->view;
            
        }
        // depth
        if(hasDepthAttachment){
            TextureAttachment* textureAttachment = get<TextureAttachment>(desc.depthAttachment.texture);
            Texture* texture =  get<Texture>(textureAttachment->textures[frame]);
            attachments[attachmentCount-1] = texture->view;
        }

        // build framebuffer create info
        VkFramebufferCreateInfo framebufferInfo {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = NULL,
            .flags           = 0,
            .renderPass      = vulkanRenderPass,
            .attachmentCount = attachmentCount,
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
        .colorAttachments   = desc.colorAttachments,
        .swapchainOverride  = swapchainOverride
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
    if (!desc.vertexShader.path.empty()){
        hasVertexShader = true;

        // get shader bytes
        std::ifstream instream(desc.vertexShader.path, std::ios::in | std::ios::binary);
        if (!instream){
            std::string message = "[VulkanResourceManager] [create<Shader>] Shader (VS) not found: ";
            message += desc.vertexShader.path;
            SprLog::error(message);
        }
        std::vector<uint8> bytes = std::vector<uint8>((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
        uint32 size = bytes.size();

        // build shader module info
        VkShaderModuleCreateInfo shaderModuleInfo {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext    = NULL,
            .flags    = 0,
            .codeSize = size,
            .pCode    = reinterpret_cast<const uint32_t*>(bytes.data())
        };

        // create shader module
        VK_CHECK(vkCreateShaderModule(m_device, &shaderModuleInfo, NULL, &vertexShader));
    }

    // create fragment shader module
    VkShaderModule fragmentShader;
    bool hasFragmentShader = false;
    if (!desc.fragmentShader.path.empty()){
        hasFragmentShader = true;

        // get shader bytes
        std::ifstream instream(desc.fragmentShader.path, std::ios::in | std::ios::binary);
        if (!instream){
            std::string message = "[VulkanResourceManager] [create<Shader>] Shader (FS) not found: ";
            message += desc.fragmentShader.path;
            SprLog::error(message);
        }
        std::vector<uint8> bytes = std::vector<uint8>((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
        uint32 size = bytes.size();

        // build shader module info
        VkShaderModuleCreateInfo shaderModuleInfo {
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext    = NULL,
            .flags    = 0,
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
    std::vector<VkDescriptorSetLayout> emptyLayouts;
    for (auto setLayoutHandle : desc.descriptorSets) {
        // check for empty layouts
        if (!setLayoutHandle.isValid()){ 
            VkDescriptorSetLayoutCreateInfo emptyLayoutInfo = {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = NULL,
                .flags = 0,
                .bindingCount = 0,
                .pBindings = NULL
            };
            VkDescriptorSetLayout emptyLayout;
            VK_CHECK(vkCreateDescriptorSetLayout(m_device, &emptyLayoutInfo, NULL, &emptyLayout));
            descriptorSetLayouts.push_back(emptyLayout);
            emptyLayouts.push_back(emptyLayout);
        } else { // valid layout
            DescriptorSetLayout* setLayout = get<DescriptorSetLayout>(setLayoutHandle);
            descriptorSetLayouts.push_back(setLayout->descriptorSetLayout);
        }
    }
    VkPipelineLayoutCreateInfo pipelineLayoutInfo {
        .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                  = NULL,
        .flags                  = 0,
        .setLayoutCount         = (uint32)descriptorSetLayouts.size(),
        .pSetLayouts            = descriptorSetLayouts.data(),
        .pushConstantRangeCount = 0,
        .pPushConstantRanges    = NULL
    };

    // create pipeline layout
    VkPipelineLayout pipelineLayout;
    VK_CHECK(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, NULL, &pipelineLayout));

    // graphics state meta
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments(renderPassLayout->colorReferences.size());
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
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = 1.0f
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
        for (int i = 0; i < renderPassLayout->colorReferences.size(); i++){
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
            .attachmentCount = (uint32)renderPassLayout->colorReferences.size(),
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
        .pNext               = NULL,
        .flags               = 0,
        .stageCount          = (uint32)(hasVertexShader + hasFragmentShader),
        .pStages             = shaderStages.data(),
        .pVertexInputState   = &vertexInputState,
        .pInputAssemblyState = &inputAssemblyState,
        .pTessellationState  = NULL,
        .pViewportState      = &viewportState,
        .pRasterizationState = &rasterizationState,
        .pMultisampleState   = &multisampleState,
        .pDepthStencilState  = &depthStencilState,
        .pColorBlendState    = &colorBlendState,
        .pDynamicState       = &dynamicState,
        .layout              = pipelineLayout,
        .renderPass          = renderPass->renderPass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = 0
    };

    // create pipeline
    VkPipeline vulkanPipeline;
    VK_CHECK(vkCreateGraphicsPipelines(m_device, NULL, 1, &pipelineInfo, NULL, &vulkanPipeline));

    // create shader resource, return handle
    Shader shader {
        .layout = pipelineLayout,
        .pipeline = vulkanPipeline,
        .emptyDescSetLayouts = emptyLayouts,
        .vertexModule = vertexShader,
        .fragmentModule = fragmentShader
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
Handle<RenderPass> VulkanResourceManager::recreate<RenderPass>(Handle<RenderPass> handle, glm::uvec2 newDimensions){
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    RenderPass* renderPass = get<RenderPass>(handle);

    // update screen dimensions
    m_screenDim = glm::uvec3(newDimensions.x, newDimensions.y, m_screenDim.z);
    renderPass->dimensions = m_screenDim;

    // layout
    Handle<RenderPassLayout> renderPassLayout = renderPass->layout;
    RenderPassLayout* layout = get<RenderPassLayout>(renderPassLayout);

    // attachment info
    bool hasDepthAttachment = renderPass->hasDepthAttachment;
    bool swapchainOverride = renderPass->swapchainOverride;
    uint32 attachmentCount = layout->attachmentCount;
    std::vector<RenderPass::ColorAttachment>& colorAttachments = renderPass->colorAttachments;
    RenderPass::DepthAttachment& depthAttachment = renderPass->depthAttachment;
    uint32 frameCount = swapchainOverride ? colorAttachments[0].swapchainImageViews.size() : MAX_FRAME_COUNT;
    
    // destroy framebuffers
    for (int i = 0; i < frameCount; i++){
        vkDestroyFramebuffer(m_device, renderPass->framebuffers[i], NULL);
    }

    // rebuild all textures that use default res,
    // assuming this renderpass doesnt use swapchain images
    if (!swapchainOverride){
        // color
        for (RenderPass::ColorAttachment& attachment : colorAttachments){
            TextureAttachment* textureAttachment = get<TextureAttachment>(attachment.texture);

            // recreate attachment's images/views
            for(int frame = 0; frame < frameCount; frame++){
                Handle<Texture> textureHandle = textureAttachment->textures[frame];
                Texture* texture = get<Texture>(textureHandle);

                // only need to create those that match screen res
                if(!texture->defaultRes)
                    continue;
                
                vkDestroyImageView(m_device, texture->view, NULL);
                vmaDestroyImage(m_allocator, texture->image, texture->alloc);

                // create image
                VkImageCreateInfo imageInfo {
                    .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .pNext       = NULL,
                    .flags       = 0,
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
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
                };
                allocate<Texture>(textureHandle, imageInfo);      

                // create image view
                VkImageViewCreateInfo viewInfo {
                    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext      = NULL,
                    .flags      = 0,
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
    
        // depth
        if (hasDepthAttachment){
            // get color attachment
            TextureAttachment* textureAttachment = get<TextureAttachment>(depthAttachment.texture);

            // recreate attachment's images/views
            for(int frame = 0; frame < frameCount; frame++){
                Handle<Texture> textureHandle = textureAttachment->textures[frame];
                Texture* texture = get<Texture>(textureHandle);

                // only need to create those that match screen res
                if(!texture->defaultRes)
                    continue;
                
                vkDestroyImageView(m_device, texture->view, NULL);
                vmaDestroyImage(m_allocator, texture->image, texture->alloc);

                // create image
                VkImageCreateInfo imageInfo {
                    .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .pNext       = NULL,
                    .flags       = 0,
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
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
                };
                allocate<Texture>(textureHandle, imageInfo);       

                // create image view
                VkImageViewCreateInfo viewInfo {
                    .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext      = NULL,
                    .flags      = 0,
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
       
    // get attachment image views
    VkImageView vulkanAttachments[frameCount][attachmentCount];
    for (int frame = 0;frame < frameCount; frame++){
        if (swapchainOverride){
            // new sc views will already be provided by the time recreate is called
            vulkanAttachments[frame][0] = colorAttachments[0].swapchainImageViews[frame];
            break;
        }
        // fetch provided attachments
        for (uint32 i = 0; i < attachmentCount+hasDepthAttachment; i++){
            TextureAttachment* textureAttachment = get<TextureAttachment>(colorAttachments[i].texture);
            Texture* texture = get<Texture>(textureAttachment->textures[frame]);
            vulkanAttachments[frame][i] = texture->view;
        }
        
    }
    if (hasDepthAttachment){
        TextureAttachment* textureAttachment = get<TextureAttachment>(depthAttachment.texture);
        for (int frame = 0; frame < frameCount; frame++){
            Texture* texture = get<Texture>(textureAttachment->textures[frame]);
            vulkanAttachments[frame][attachmentCount-1] = texture->view;
        }
    }

    // rebuild framebuffers
    std::vector<VkFramebuffer>& vulkanFramebuffers = renderPass->framebuffers;
    for (int frame = 0; frame < frameCount; frame++){
        // build framebuffer create info
        VkFramebufferCreateInfo framebufferInfo {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext           = NULL,
            .flags           = 0,
            .renderPass      = renderPass->renderPass,
            .attachmentCount = attachmentCount,
            .pAttachments    = vulkanAttachments[frame],
            .width           = newDimensions.x,
            .height          = newDimensions.y,
            .layers          = 1
        };

        // create vulkan framebuffer, directly overwriting
        // existing framebuffers in RenderPass object
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
    Buffer* buffer = get<Buffer>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        vmaDestroyBuffer(m_allocator, buffer->buffer, buffer->alloc);
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 Texture                                                   //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<Texture>(Handle<Texture> handle){
    TextureCache* resourceCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    Texture* texture = get<Texture>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        vkDestroySampler(m_device, texture->sampler, nullptr);
        vkDestroyImageView(m_device, texture->view, nullptr);
        vmaDestroyImage(m_allocator, texture->image, texture->alloc);
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 Texture Attachment                                        //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<TextureAttachment>(Handle<TextureAttachment> handle){
    TextureAttachmentCache* resourceCache = ((TextureAttachmentCache*) m_resourceMap[typeid(TextureAttachment)]);
    TextureAttachment* textureAttachment = get<TextureAttachment>(handle);

    for(Handle<Texture> textureHandle : textureAttachment->textures){
        remove<Texture>(textureHandle);
    }
    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 DescriptorSetLayout                                       //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<DescriptorSetLayout>(Handle<DescriptorSetLayout> handle){
    DescriptorSetLayoutCache* resourceCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
    DescriptorSetLayout* descriptorSetLayout = get<DescriptorSetLayout>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        vkDestroyDescriptorSetLayout(m_device, descriptorSetLayout->descriptorSetLayout, nullptr);
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 DescriptorSet                                             //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<DescriptorSet>(Handle<DescriptorSet> handle){
    DescriptorSetCache* resourceCache = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
    DescriptorSet* descriptorSet = get<DescriptorSet>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        // descriptor set destroyed w/ descriptor pool
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 RenderPassLayout                                          //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<RenderPassLayout>(Handle<RenderPassLayout> handle){
    RenderPassLayoutCache* resourceCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);
    RenderPassLayout* renderPassLayout = get<RenderPassLayout>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 RenderPass                                                //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<RenderPass>(Handle<RenderPass> handle){
    RenderPassCache* resourceCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    RenderPass* renderPass = get<RenderPass>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        for (VkFramebuffer framebuffer : renderPass->framebuffers){
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }
        vkDestroyRenderPass(m_device, renderPass->renderPass, nullptr);
        resourceCache->remove(handle);
    });
};


// ------------------------------------------------------------------------- //
//                 Shader                                                    //
// ------------------------------------------------------------------------- //
template<>
void VulkanResourceManager::remove<Shader>(Handle<Shader> handle){
    ShaderCache* resourceCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
    Shader* shader = get<Shader>(handle);

    m_deletionQueue[m_frameId % MAX_FRAME_COUNT].push_function([=]() {
        vkDestroyPipeline(m_device, shader->pipeline, nullptr);
        vkDestroyPipelineLayout(m_device, shader->layout, nullptr);
        vkDestroyShaderModule(m_device, shader->vertexModule, nullptr);
        vkDestroyShaderModule(m_device, shader->fragmentModule, nullptr);
        for (VkDescriptorSetLayout emptyLayout : shader->emptyDescSetLayouts)
            vkDestroyDescriptorSetLayout(m_device, emptyLayout, nullptr);
        resourceCache->remove(handle);
    });
};



//  ██╗  ██╗███████╗██╗     ██████╗ ███████╗██████╗ 
//  ██║  ██║██╔════╝██║     ██╔══██╗██╔════╝██╔══██╗
//  ███████║█████╗  ██║     ██████╔╝█████╗  ██████╔╝
//  ██╔══██║██╔══╝  ██║     ██╔═══╝ ██╔══╝  ██╔══██╗
//  ██║  ██║███████╗███████╗██║     ███████╗██║  ██║
//  ╚═╝  ╚═╝╚══════╝╚══════╝╚═╝     ╚══════╝╚═╝  ╚═╝

void VulkanResourceManager::flushDeletionQueue(uint32 frameId){
    m_frameId = frameId;
    m_deletionQueue[frameId % MAX_FRAME_COUNT].execute();
}
                                                

}