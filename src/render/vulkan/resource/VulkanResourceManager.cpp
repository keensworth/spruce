#include "VulkanResourceManager.h"
#include "ResourceFlags.h"
#include "ResourceTypes.h"
#include <vulkan/vulkan_core.h>
#include <fstream>

namespace spr::gfx {


//  ██╗███╗  ██╗██╗████████╗
//  ██║████╗ ██║██║╚══██╔══╝
//  ██║██╔██╗██║██║   ██║   
//  ██║██║╚████║██║   ██║   
//  ██║██║ ╚███║██║   ██║   
//  ╚═╝╚═╝  ╚══╝╚═╝   ╚═╝   

VulkanResourceManager::VulkanResourceManager(){

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

    // create buffer
    VkBuffer vulkankBuffer;
    VK_CHECK(vkCreateBuffer(m_device, &bufferInfo, NULL, &vulkankBuffer));

    Buffer buffer;
    return bufferCache->insert(buffer);
}


// ------------------------------------------------------------------------- //
//                 Texture                                                   //
// ------------------------------------------------------------------------- //
template<>
Handle<Texture> VulkanResourceManager::create<Texture>(TextureDesc desc){
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);

    // build image create info
    VkImageCreateInfo imageInfo {
        .sType       = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType   = VK_IMAGE_TYPE_2D,
        .format      = (VkFormat)desc.format,
        .extent      = {
            desc.dimensions.x,
            desc.dimensions.y,
            desc.dimensions.z
        },
        .mipLevels   = desc.view.mips,
        .arrayLayers = desc.view.layers,
        .samples     = (VkSampleCountFlagBits)desc.samples,
        .tiling      = VK_IMAGE_TILING_OPTIMAL,
        .usage       = (VkImageUsageFlags)desc.usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    // create image
    VkImage vulkanImage;
    VK_CHECK(vkCreateImage(m_device, &imageInfo, NULL, &vulkanImage));

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
    VkSampler vulkanSampler;
    VK_CHECK(vkCreateSampler(m_device, &samplerInfo, NULL, &vulkanSampler));

    // create image view (default)
    VkImageViewCreateInfo viewInfo {
        .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image      = vulkanImage,
        .viewType   = VK_IMAGE_VIEW_TYPE_2D,
        .format     = (VkFormat)desc.format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_R,
            .g = VK_COMPONENT_SWIZZLE_G,
            .b = VK_COMPONENT_SWIZZLE_B,
            .a = VK_COMPONENT_SWIZZLE_A
        },
        .subresourceRange = {
            .aspectMask = (desc.usage == Flags::ImageUsage::IU_DEPTH_STENCIL_ATTACHMENT)
                           ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = desc.view.baseMip,
            .levelCount = desc.view.mips,
            .baseArrayLayer = desc.view.baseLayer,
            .layerCount = desc.view.layers
        }
    };
    VkImageView vulkanImageView;
    VK_CHECK(vkCreateImageView(m_device, &viewInfo, NULL, &vulkanImageView));

    // crate texture resource, return handle
    Texture texture {
        .image = vulkanImage,
        .view = vulkanImageView,
        .sampler = vulkanSampler,
    };
    return textureCache->insert(texture);
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
        .textureLayouts = desc.textures,
        .bufferLayouts = desc.buffers,
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

    // build descriptor set info
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = NULL, // TODO: pass in pool
        .descriptorSetCount = 1,
        .pSetLayouts = &(layout->descriptorSetLayout)
    };

    // allocate descriptor sets
    VkDescriptorSet vulkanDescriptorSet;
    VK_CHECK(vkAllocateDescriptorSets(m_device, NULL, &vulkanDescriptorSet));

    // write descriptor sets
    {
        // buffers
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

        // textures
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
        .descriptorSet = vulkanDescriptorSet
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
                .format = (VkFormat)desc.colorAttatchmentFormats[i]
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

    // build RenderPassLayout, return handle
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
    
    // insert new description info into descriptions
    uint32 samples;
    bool depthAttachment = (layout->colorReferences.size() + 1) == layout->attachmentCount;
    // depth
    if (depthAttachment) {
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

    // build RenderPass, return handle
    RenderPass renderPass {
        .layout = desc.layout,
        .renderPass = vulkanRenderPass,
        .samples = samples
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

    Shader shader {
        .pipeline = vulkanPipeline
    };
    return shaderCache->insert(shader);
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