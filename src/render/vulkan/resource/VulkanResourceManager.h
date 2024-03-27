#pragma once

#include <typeindex>
#include "VulkanResourceCache.h"
#include "util/FunctionQueue.h"
#include "../debug/SprLog.h"

namespace spr::gfx {

class VulkanDevice;

class BufferCache : public TypedResourceCache<Buffer>{};
class TextureCache : public TypedResourceCache<Texture>{};
class TextureAttachmentCache : public TypedResourceCache<TextureAttachment>{};
class DescriptorSetLayoutCache : public TypedResourceCache<DescriptorSetLayout>{};
class DescriptorSetCache : public TypedResourceCache<DescriptorSet>{};
class FramebufferCache : public TypedResourceCache<Framebuffer>{};
class RenderPassLayoutCache : public TypedResourceCache<RenderPassLayout>{};
class RenderPassCache : public TypedResourceCache<RenderPass>{};
class ShaderCache : public TypedResourceCache<Shader>{};

typedef ska::flat_hash_map<std::type_index, VulkanResourceCache*> rmap;

class VulkanResourceManager {
public:
    VulkanResourceManager();
    ~VulkanResourceManager();

    void init(VulkanDevice& device, glm::uvec3 screenDim);
    void destroy();

    void updateScreenDim(glm::uvec2 screenDim){
        m_screenDim = {screenDim.x, screenDim.y, 1};
    }

    // U := ResourceType
    template <typename U>
    U* get(Handle<U> handle){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->get(handle);
    }

    // U := ResourceType
    template <typename U>
    void remove(Handle<U> handle){
        SprLog::warn("[VulkanResourceManager] [REMOVE] Resource may not be properly destroyed");
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->remove(handle);
    }

    // U := ResourceType
    // V := ResourceDesc
    template <typename U>
    Handle<U> create(typename U::Desc desc){
        SprLog::warn("[VulkanResourceManager] [CREATE] Resource not recognized");
        return Handle<U>();
    }

    // U := ResourceType
    // V := (unknown)
    template <typename U, typename V>
    Handle<U> recreate(Handle<U> handle, V arg){
        SprLog::warn("[VulkanResourceManager] [RECREATE] Resource recreation not available for this type, excplicit specialization required");
        return Handle<U>();
    }

    uint32 alignedSize(size_t typeSize){
        size_t alignedSize = typeSize;
        if (m_minUBOAlignment > 0) {
            alignedSize = (alignedSize + m_minUBOAlignment - 1) & ~(m_minUBOAlignment - 1);
        }
        return alignedSize;
    }


private:
    // U := ResourceType
    template <typename U, typename V>
    void allocate(Handle<U> handle, V& info){
        SprLog::warn("[VulkanResourceManager] [ALLOCATE] Resource not recognized");
        return;
    }

    void flushDeletionQueue(uint32 frameId);

    rmap m_resourceMap{
        {typeid(Buffer),              new BufferCache},
        {typeid(Texture),             new TextureCache},
        {typeid(TextureAttachment),   new TextureAttachmentCache},
        {typeid(DescriptorSetLayout), new DescriptorSetLayoutCache},
        {typeid(DescriptorSet),       new DescriptorSetCache},
        {typeid(Framebuffer),         new FramebufferCache},
        {typeid(RenderPassLayout),    new RenderPassLayoutCache},
        {typeid(RenderPass),          new RenderPassCache},
        {typeid(Shader),              new ShaderCache},
    };

private: // owning
    VmaAllocator m_allocator;
    VkDescriptorPool m_globalDescriptorPool;
    VkDescriptorPool m_dynamicDescriptorPools[MAX_FRAME_COUNT]; 
    FunctionQueue m_deletionQueue[MAX_FRAME_COUNT];

private: // non-owning
    VkDevice m_device;
    uint32 m_minUBOAlignment;
    glm::uvec3 m_screenDim;
    bool m_destroyed = false;
    uint32 m_frameId = 0;

    friend class VulkanRenderer;
    friend class RenderPassRenderer;
};


template<> void VulkanResourceManager::allocate<Buffer>(Handle<Buffer> handle, VkBufferCreateInfo& info);
template<> void VulkanResourceManager::allocate<Texture>(Handle<Texture> handle, VkImageCreateInfo& info);

template<> Handle<Buffer> VulkanResourceManager::create<Buffer>(BufferDesc desc);
template<> Handle<Texture> VulkanResourceManager::create<Texture>(TextureDesc desc);
template<> Handle<TextureAttachment> VulkanResourceManager::create<TextureAttachment>(TextureAttachmentDesc desc);
template<> Handle<DescriptorSetLayout> VulkanResourceManager::create<DescriptorSetLayout>(DescriptorSetLayoutDesc desc);
template<> Handle<DescriptorSet> VulkanResourceManager::create<DescriptorSet>(DescriptorSetDesc desc);
template<> Handle<Framebuffer> VulkanResourceManager::create<Framebuffer>(FramebufferDesc desc);
template<> Handle<RenderPassLayout> VulkanResourceManager::create<RenderPassLayout>(RenderPassLayoutDesc desc);
template<> Handle<RenderPass> VulkanResourceManager::create<RenderPass>(RenderPassDesc desc);
template<> Handle<Shader> VulkanResourceManager::create<Shader>(ShaderDesc desc);

template<> Handle<Framebuffer> VulkanResourceManager::recreate<Framebuffer>(Handle<Framebuffer> handle, FramebufferDesc desc);
template<> Handle<RenderPass> VulkanResourceManager::recreate<RenderPass>(Handle<RenderPass> handle, glm::uvec2 newDimensions);
template<> Handle<Shader> VulkanResourceManager::recreate<Shader>(Handle<Shader> handle, bool temp);

template<> void VulkanResourceManager::remove<Buffer>(Handle<Buffer> handle);
template<> void VulkanResourceManager::remove<Texture>(Handle<Texture> handle);
template<> void VulkanResourceManager::remove<TextureAttachment>(Handle<TextureAttachment> handle);
template<> void VulkanResourceManager::remove<DescriptorSetLayout>(Handle<DescriptorSetLayout> handle);
template<> void VulkanResourceManager::remove<DescriptorSet>(Handle<DescriptorSet> handle);
template<> void VulkanResourceManager::remove<Framebuffer>(Handle<Framebuffer> handle);
template<> void VulkanResourceManager::remove<RenderPassLayout>(Handle<RenderPassLayout> handle);
template<> void VulkanResourceManager::remove<RenderPass>(Handle<RenderPass> handle);
template<> void VulkanResourceManager::remove<Shader>(Handle<Shader> handle);
}