#include "VulkanResourceManager.h"
#include "ResourceFlags.h"
#include "ResourceTypes.h"
#include <vulkan/vulkan_core.h>

namespace spr::gfx {


//  ██╗███╗░░██╗██╗████████╗
//  ██║████╗░██║██║╚══██╔══╝
//  ██║██╔██╗██║██║░░░██║░░░
//  ██║██║╚████║██║░░░██║░░░
//  ██║██║░╚███║██║░░░██║░░░
//  ╚═╝╚═╝░░╚══╝╚═╝░░░╚═╝░░░

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


//  ░█████╗░██████╗░███████╗░█████╗░████████╗███████╗
//  ██╔══██╗██╔══██╗██╔════╝██╔══██╗╚══██╔══╝██╔════╝
//  ██║░░╚═╝██████╔╝█████╗░░███████║░░░██║░░░█████╗░░
//  ██║░░██╗██╔══██╗██╔══╝░░██╔══██║░░░██║░░░██╔══╝░░
//  ╚█████╔╝██║░░██║███████╗██║░░██║░░░██║░░░███████╗
//  ░╚════╝░╚═╝░░╚═╝╚══════╝╚═╝░░╚═╝░░░╚═╝░░░╚══════╝

// ------------------------------------------------------------------------- //
//                 Buffer                                                    // 
// ------------------------------------------------------------------------- //
template<>
Handle<Buffer> VulkanResourceManager::create<Buffer>(BufferDesc bufferDesc){
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
    Buffer buffer;
    return bufferCache->insert(buffer);
}


// ------------------------------------------------------------------------- //
//                 Texture                                                   // 
// ------------------------------------------------------------------------- //
template<>
Handle<Texture> VulkanResourceManager::create<Texture>(TextureDesc textureDesc){
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    Texture texture;
    return textureCache->insert(texture);
}


// ------------------------------------------------------------------------- //
//                 Descriptor Set Layout                                     // 
// ------------------------------------------------------------------------- //
template<>
Handle<DescriptorSetLayout> VulkanResourceManager::create<DescriptorSetLayout>(DescriptorSetLayoutDesc desc){
    DescriptorSetLayoutCache* descriptorSetLayoutCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
    DescriptorSetLayout descriptorSetLayout;
    return descriptorSetLayoutCache->insert(descriptorSetLayout);
}


// ------------------------------------------------------------------------- //
//                 Descriptor Set                                            // 
// ------------------------------------------------------------------------- //
template<>
Handle<DescriptorSet> VulkanResourceManager::create<DescriptorSet>(DescriptorSetDesc descriptorSetDesc){
    DescriptorSetCache* descriptorSetCache = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
    DescriptorSet descriptorSet;
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

    // build RenderPassLayout 
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

    // build RenderPass
    RenderPass renderPass {
        .layout = desc.layout,
        .renderPass = vulkanRenderPass
    };
    return renderPassCache->insert(renderPass);
}


// ------------------------------------------------------------------------- //
//                 Shader                                                    // 
// ------------------------------------------------------------------------- //
template<>
Handle<Shader> VulkanResourceManager::create<Shader>(ShaderDesc shaderDesc){
    ShaderCache* shaderCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
    Shader shader;
    return shaderCache->insert(shader);
}


//  ██████╗░███████╗███╗░░░███╗░█████╗░██╗░░░██╗███████╗
//  ██╔══██╗██╔════╝████╗░████║██╔══██╗██║░░░██║██╔════╝
//  ██████╔╝█████╗░░██╔████╔██║██║░░██║╚██╗░██╔╝█████╗░░
//  ██╔══██╗██╔══╝░░██║╚██╔╝██║██║░░██║░╚████╔╝░██╔══╝░░
//  ██║░░██║███████╗██║░╚═╝░██║╚█████╔╝░░╚██╔╝░░███████╗
//  ╚═╝░░╚═╝╚══════╝╚═╝░░░░░╚═╝░╚════╝░░░░╚═╝░░░╚══════╝


}