#pragma once

#include <typeindex>
#include "VulkanResourceCache.h"


typedef ska::flat_hash_map<std::type_index, spr::gfx::VulkanResourceCache*> rmap;

namespace spr::gfx {

class ShaderCache : public TypedResourceCache<Shader>{};
class PipelineCache : public TypedResourceCache<Pipeline>{};
class BufferCache : public TypedResourceCache<Buffer>{};
class TextureCache : public TypedResourceCache<Texture>{};
class MeshCache : public TypedResourceCache<Mesh>{};
class DescriptorSetLayoutCache : public TypedResourceCache<DescriptorSetLayout>{};
class DescriptorSetCache : public TypedResourceCache<DescriptorSet>{};
class RenderPassLayoutCache : public TypedResourceCache<RenderPassLayout>{};
class RenderPassCache : public TypedResourceCache<RenderPass>{};
class FrameBufferCache : public TypedResourceCache<FrameBuffer>{};


class VulkanResourceManager {
public:
    VulkanResourceManager();
    ~VulkanResourceManager();

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
        auto resourceCache = m_resourceMap[typeid(U)];
        auto typedCache = dynamic_cast<TypedResourceCache<U>*>(resourceCache);
        return typedCache->remove(handle);
    }

    // U := ResourceType
    // V := ResourceDesc
    template <typename U, typename V>
    Handle<U> create(V desc){
        SprLog::warn("VulkanResourceManager: Resource not recognized");
        return Handle<U>();
    }
    
private:
    rmap m_resourceMap{
        {typeid(Shader), new ShaderCache},
        {typeid(Pipeline), new PipelineCache},
        {typeid(Buffer), new BufferCache},
        {typeid(Texture), new TextureCache},
        {typeid(Mesh), new MeshCache},
        {typeid(DescriptorSetLayout), new DescriptorSetLayoutCache},
        {typeid(DescriptorSet), new DescriptorSetCache},
        {typeid(RenderPassLayout), new RenderPassLayoutCache},
        {typeid(RenderPass), new RenderPassCache},
        {typeid(FrameBuffer), new FrameBufferCache},
    };

    void init();
};
}