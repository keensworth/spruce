#include "VulkanResourceManager.h"

namespace spr::gfx {

VulkanResourceManager::VulkanResourceManager(){

}

VulkanResourceManager::~VulkanResourceManager(){
    // delete shader cache
    ShaderCache* shaderCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
    delete shaderCache;

    // delete pipeline cache
    PipelineCache* pipelineCache = ((PipelineCache*) m_resourceMap[typeid(Pipeline)]);
    delete pipelineCache;

    // delete buffer cache
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
    delete bufferCache;

    // delete texture cache
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    delete textureCache;

    // delete mesh cache
    MeshCache* meshCache = ((MeshCache*) m_resourceMap[typeid(Mesh)]);
    delete meshCache;

    // delete descriptor set layout cache
    DescriptorSetLayoutCache* descriptorSetLayoutCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
    delete descriptorSetLayoutCache;

    // delete descriptor set cache
    DescriptorSetCache* descriptorSet = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
    delete descriptorSet;

    // delete render pass layout cache
    RenderPassLayoutCache* renderPassLaoutCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);
    delete renderPassLaoutCache;

    // delete render pass cache
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    delete renderPassCache;

    // delete frame buffer cache
    FrameBufferCache* frameBufferCache = ((FrameBufferCache*) m_resourceMap[typeid(FrameBuffer)]);
    delete frameBufferCache;
}

template<>
Handle<Shader> VulkanResourceManager::create<Shader>(ShaderDesc shaderDesc){
    ShaderCache* shaderCache = ((ShaderCache*) m_resourceMap[typeid(Shader)]);
    Shader shader;
    return shaderCache->insert(shader);
}

template<>
Handle<Pipeline> VulkanResourceManager::create<Pipeline>(PipelineDesc pipelineDesc){
    PipelineCache* pipelineCache = ((PipelineCache*) m_resourceMap[typeid(Pipeline)]);
    Pipeline pipeline;
    return pipelineCache->insert(pipeline);
}

template<>
Handle<Buffer> VulkanResourceManager::create<Buffer>(BufferDesc bufferDesc){
    BufferCache* bufferCache = ((BufferCache*) m_resourceMap[typeid(Buffer)]);
    Buffer buffer;
    return bufferCache->insert(buffer);
}

template<>
Handle<Texture> VulkanResourceManager::create<Texture>(TextureDesc textureDesc){
    TextureCache* textureCache = ((TextureCache*) m_resourceMap[typeid(Texture)]);
    Texture texture;
    return textureCache->insert(texture);
}

template<>
Handle<Mesh> VulkanResourceManager::create<Mesh>(MeshDesc meshDesc){
    MeshCache* meshCache = ((MeshCache*) m_resourceMap[typeid(Mesh)]);
    Mesh mesh;
    return meshCache->insert(mesh);
}

template<>
Handle<DescriptorSetLayout> VulkanResourceManager::create<DescriptorSetLayout>(DescriptorSetLayoutDesc descriptorSetLayoutDesc){
    DescriptorSetLayoutCache* descriptorSetLayoutCache = ((DescriptorSetLayoutCache*) m_resourceMap[typeid(DescriptorSetLayout)]);
    DescriptorSetLayout descriptorSetLayout;
    return descriptorSetLayoutCache->insert(descriptorSetLayout);
}

template<>
Handle<DescriptorSet> VulkanResourceManager::create<DescriptorSet>(DescriptorSetDesc descriptorSetDesc){
    DescriptorSetCache* descriptorSetCache = ((DescriptorSetCache*) m_resourceMap[typeid(DescriptorSet)]);
    DescriptorSet descriptorSet;
    return descriptorSetCache->insert(descriptorSet);
}

template<>
Handle<RenderPassLayout> VulkanResourceManager::create<RenderPassLayout>(RenderPassLayoutDesc renderPassLayoutDesc){
    RenderPassLayoutCache* renderPassLaoutCache = ((RenderPassLayoutCache*) m_resourceMap[typeid(RenderPassLayout)]);
    RenderPassLayout renderPassLayout;
    return renderPassLaoutCache->insert(renderPassLayout);
}

template<>
Handle<RenderPass> VulkanResourceManager::create<RenderPass>(RenderPassDesc renderPassDesc){
    RenderPassCache* renderPassCache = ((RenderPassCache*) m_resourceMap[typeid(RenderPass)]);
    RenderPass renderPass;
    return renderPassCache->insert(renderPass);
}

template<>
Handle<FrameBuffer> VulkanResourceManager::create<FrameBuffer>(FrameBufferDesc frameBufferDesc){
    FrameBufferCache* frameBufferCache = ((FrameBufferCache*) m_resourceMap[typeid(FrameBuffer)]);
    FrameBuffer frameBuffer;
    return frameBufferCache->insert(frameBuffer);
}

}