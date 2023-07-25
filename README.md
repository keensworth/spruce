# spruce
`spruce` is an exploration into Vulkan and C++20, with some additional pleasantries on top. Currently, abstractions are provided for rendering, resource management, ecs, and windowing/input. See usage information [here](#usage).
Currently just a personal project.

## Renderer
### Features
The renderer currently supports the following:
- PBR materials, lighting (w/ acceptable defaults)
- Ground Truth Ambient Occlusion (GTAO)
- Cascaded shadow mapping
- Instancing by mesh/material batch
- Mipmapping
- Cubemaps
- Normal mapping
- Shader reloading
- Deferred resource destruction
- Transfer queue
- Dear ImGui interface

The renderer also uses a handle-based resource manager, which abstracts the creation and deletion of:
- Textures + attachments
- Buffers
- Descriptor sets + layouts
- Render passes
- Shaders (pipelines)

This makes it trivial to create and order the execution of different passes, and thread resources between them. Descriptor sets are only rebound if they change between subsequent render passes. Resources can be bound to descriptor sets in various ways, such as global (shared over frames), per-frame (resource per frame), dynamic (single resource, suballocated per-frame), and array (of unique textures or buffers).

### Output Samples
[`PBR + CSM + GTAO (#1)`](https://github.com/keensworth/spruce/blob/main/src/render/renderers/LitMeshRenderer.h)
![fullpass1](https://i.imgur.com/r6cOnkf.png)
[`PBR + CSM + GTAO (#2)`](https://github.com/keensworth/spruce/blob/main/src/render/renderers/LitMeshRenderer.h)
![fullpass2](https://i.imgur.com/SSsUFWN.png)

[`PBR + CSM + GTAO (#3)`](https://github.com/keensworth/spruce/blob/main/src/render/renderers/LitMeshRenderer.h)
![fullpass3](https://i.imgur.com/fj9ixfh.png)

[`GTAO (unfiltered)`](https://github.com/keensworth/spruce/blob/main/src/render/renderers/GTAORenderer.h)
![gtao](https://i.imgur.com/6pCKHmH.png)

[`Shadow Cascade`](https://github.com/keensworth/spruce/blob/main/src/render/renderers/SunShadowRenderer.h)
![csm](https://i.imgur.com/MI6hQ6E.png)

[`Debug Normals`](https://github.com/keensworth/spruce/blob/main/src/render/renderers/DebugNormalsRenderer.h)
![normals](https://i.imgur.com/SWQ0lMA.png)

### API (gfx)
The gfx-user-land API uses C++20 designated initializers to create render resources. Underneath, these are backed by a custom span type that accepts initializer lists. This allows concise and structured construction of resources (with an unknown or varying number of parameters) without relying on a dynamic memory backed type such as `std::vector`. 

Creation of resources returns a handle to that resource (such as `Handle<Buffer>`), which is just two integers: an index into that resource type's array, and a generation number (which increments as a new resource is created at a given index) to verify the resource is still valid.

 ```cpp
// buffer
Handle<Buffer> buffer = rm->create<Buffer>({
    .byteSize = (uint32) (count * sizeof(ExampleStruct)),
    .usage = Flags::BufferUsage::BU_STORAGE_BUFFER |
             Flags::BufferUsage::BU_TRANSFER_DST,
    .memType = DEVICE
});

// texture (sampled)
Handle<Texture> texture = rm->create<Texture>({
    .dimensions = {
        width,
        height,
        1
    },
    .format = Flags::Format::RGBA8_UNORM,
    .usage = Flags::ImageUsage::IU_SAMPLED |
             Flags::ImageUsage::IU_TRANSFER_DST
});

// attachment (render target)
Handle<Attachment> attachment = rm->create<Attachment>({
    .textureLayout = {
        .dimensions = screenDim,
        .format = Flags::Format::RGBA8_UNORM,
        .usage = Flags::ImageUsage::IU_COLOR_ATTACHMENT |
                 Flags::ImageUsage::IU_SAMPLED
    }
});

// descriptor set + layout
Handle<DescriptorSetLayout> descSetLayout = rm->create<DescriptorSetLayout>({
    .textures = {
        {.binding = 2} // attachment as input
        {.binding = 3} // texture
        {.binding = 4, .count = textureCount}, // array of textures
    },
    .buffers = {
        {.binding = 0, .type = Flags::DescriptorType::STORAGE_BUFFER},
        {.binding = 1, .type = Flags::DescriptorType::UNIFORM_BUFFER}
    }
});

Handle<DescriptorSet> descSet = rm->create<DescriptorSet>({
    .textures = {
        {.attachment = attachmentHandle},
        {.texture = textureHandle},
        {.textures = {texHandles, texCount}}
    },
    .buffers = {
        {.buffer = storageBufferHandle},
        {.buffer = uniformBufferHandle},
    },
    .layout = descSetLayout
});

// render pass + layout
Handle<RenderPassLayout> renderPassLayout = rm->create<RenderPassLayout>({
    .depthAttachmentFormat = Flags::D32_SFLOAT,
    .colorAttatchmentFormats = {Flags::RGBA8_UNORM},
    .subpass = {
        .depthAttachment = 1,
        .colorAttachments = {0}
    }
});

Handle<RenderPass> renderPass = rm->create<RenderPass>({
    .dimensions = screenDim,
    .layout = renderPassLayout,
    .depthAttachment = {
        .texture = depthAttachmentHandle,
        .loadOp = Flags::LoadOp::LOAD,
        .layout = Flags::ImageLayout::READ_ONLY,
        .finalLayout = Flags::ImageLayout::READ_ONLY
    },
    .colorAttachments = {
        {
            .texture = attachmentHandle,
            .finalLayout = Flags::ImageLayout::ATTACHMENT
        }
    }
});

// shader (pipeline)
Handle<Shader> shader = rm->create<Shader>({
    .vertexShader = {.path = "../data/shaders/spv/example_shader.vert.spv"},
    .fragmentShader = {.path = "../data/shaders/spv/example_shader.frag.spv"},
    .descriptorSets = {
        { globalDescSetLayout },
        { frameDescSetLayout },
        { descSetLayout },
        { } // unused
    },
    .graphicsState = {
        .depthTest = Flags::Compare::GREATER_OR_EQUAL,
        .depthWriteEnabled = false,
        .renderPass = renderPass
    }
});
```

To retrieve a pointer to the resource:
 ```cpp
Buffer* buffer = rm->get<Buffer>(handle);
...
   ```

Uploading resources (buffers, textures) to device is done with the `UploadHandler`, which provides the following:
 ```cpp
void uploadBuffer(TempBuffer<T>& src, Handle<Buffer> dst);
void uploadDyanmicBuffer(TempBuffer<T>& src, Handle<Buffer> dst);
void uploadTexture(TempBuffer<T>& src, Handle<Texture> dst);
void submit();
   ```

For example resource creation, see any [pass renderer](https://github.com/keensworth/spruce/tree/main/src/render/renderers). For resource descriptions, see [ResourceTypes.h](https://github.com/keensworth/spruce/blob/main/src/render/vulkan/resource/ResourceTypes.h).

### API (high-level)
Currently, the renderer exposes limited functionality at the highest level. After loading and registering assets (see below), a user is able to interface with the renderer in the following ways:
- Insert models
- Insert meshes (perhaps particular meshes of a given model)
- Insert various light types
- Update the camera

A user can insert models and meshes with any combination of shared transforms or material overrides. For example, insertion of a model to be drawn is just an insertion of its meshes with a shared transform. Material overrides allow a user to manually enable/disable material flags beyond those inherited from the original model.

To view the high-level renderer API, see [SprRenderer.h](https://github.com/keensworth/spruce/blob/main/src/render/SprRenderer.h).

### Assets
Assets are pre-conditioned from existing images and glTF2.0 models. These are stored into a custom, simpler format for eventual usage at runtime. They are then registered, mapping every model and non-subresource image name to the corresponding id as enums to be referenced in user-land (i.e. to render `data::sponza` or establish the fallback color texture as `data::default_color`).

## Usage
### Prerequisites
- CMake >= 3.12
- C++20 compatible compiler
- SDL2
- Vulkan SDK (min 1.2 support)
- Vulkan validation layers
- (optional) SPIR-V compiler (such as `glslc`)

### Build
1. Clone the repository:
 ```
git clone https://github.com/keensworth/spruce.git
```

2. `cd` into the repository and build the project:
 ```
cd build
cmake ..
make
```

3. Run the executable:
 ```
./spruce
```

### Load Assets
To load assets (models, textures) for use at runtime, several tools are provided:
- `gltfparser`: loads valid glTF2.0 models and parses them into a simpler runtime format
- `assetcreate`: loads non-subresource textures and cubemaps
- `register_assets`: takes into account all loaded models and textures, assigns them unique ids, and updates a header file (`asset_ids.h`) -  stores the names of models and non-subresource textures as enums for use in code

`register_assets` must be run after loading/updating any models/textures.

## Future Work
### Renderer
- Properly filtered environment cubemaps for IBL 
- Compute based frustum/object culling
- Point shadows
- Volumetric lighting
- Temporal filtering (AO/AA)
- Reflections
- Separate renderer thread
- Multi-threaded command recording

## Credits / Resources
- [`Sebastian Aaltonen`](https://twitter.com/SebAaltonen) Heavy inspiration for the gfx resource API and excellent source of information on Twitter
- [`Sascha Willems`](https://twitter.com/SaschaWillems2) Large source of Vulkan examples, and large presence answering questions elsewhere
- [`Alexander Overvoorde`](https://twitter.com/overv) Creator of [Vulkan Triangle](https://vulkan-tutorial.com/), found elsewhere providing helpful insights
- [`Arseny Kapoulkine`](https://twitter.com/zeuxcg) Great [blog](https://zeux.io/2020/02/27/writing-an-efficient-vulkan-renderer/), useful breakdown of mental and practical models for descriptor sets, command recording, and pipelines/renderpasses - additionally,  [volk](https://github.com/zeux/volk)
- [`Hans-Kristian Arntzen (Maister)`](https://themaister.net/blog/) Another great blog covering topics such as renderer architecture and synchronization
- [`KhrnosGroup`](https://www.khronos.org/) Self explanatory - swathe of examples, such as [resource synchronization](https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples), [vertex input](https://github.com/KhronosGroup/Vulkan-Guide/blob/main/chapters/vertex_input_data_processing.adoc), [sample glTF2 models](https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0)
- [`Vulkan Memory Allocator`](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/index.html) Vulkan memory management abstraction, with useful articles covering [memory types](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/choosing_memory_type.html), [memory mapping](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/memory_mapping.html), and [usage patterns](https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/usage_patterns.html)
- [`Adam Sawicki`](https://asawicki.info/index) Articles discussing common [memory models](https://asawicki.info/news_1740_vulkan_memory_types_on_pc_and_how_to_use_them) for various groups of hardware, and an overview of various [Vulkan objects](https://gpuopen.com/learn/understanding-vulkan-objects/)
- [`NVIDIA: Vulkan Dos and Don'ts`](https://developer.nvidia.com/blog/vulkan-dos-donts/) Article covering ideal usage patterns when working with Vulkan

