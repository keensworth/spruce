#pragma once

#include <typeindex>
#include <vulkan/vulkan_core.h>
#include "VulkanResourceCache.h"
#include "../VulkanDevice.h"
#include "util/FunctionQueue.h"


typedef ska::flat_hash_map<std::type_index, spr::gfx::VulkanResourceCache*> rmap;

namespace spr::gfx {


class BufferCache : public TypedResourceCache<Buffer>{};
class TextureCache : public TypedResourceCache<Texture>{};
class TextureAttachmentCache : public TypedResourceCache<TextureAttachment>{};
class DescriptorSetLayoutCache : public TypedResourceCache<DescriptorSetLayout>{};
class DescriptorSetCache : public TypedResourceCache<DescriptorSet>{};
class RenderPassLayoutCache : public TypedResourceCache<RenderPassLayout>{};
class RenderPassCache : public TypedResourceCache<RenderPass>{};
class ShaderCache : public TypedResourceCache<Shader>{};


class VulkanResourceManager {
public:
    VulkanResourceManager();
    ~VulkanResourceManager();

    void init(VulkanDevice& device);
    void destroy();

    // U := ResourceType
    template <typename U>
    U* get(Handle<U> handle){
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->getData(handle);
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
    template <typename U, typename V>
    Handle<U> create(V desc){
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
    glm::uvec3 m_screenDim;
    bool m_destroyed = false;
    uint32 m_frameId = 0;

    friend class VulkanRenderer;
    friend class RenderPassRenderer;
};
}