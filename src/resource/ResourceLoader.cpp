#include "ResourceLoader.h"
#include "../debug/SprLog.h"
#include <cstring>

namespace spr{

ResourceLoader::ResourceLoader(){

}

template <typename T>
void ResourceLoader::loadFromMetadata(ResourceMetadata metadata, T& data){
    // log unknown resource
    SprLog::error("[ResourceLoader] Unkown resource");
}


// ------------------------------------------------------------------------- //
//    Model - .smdl                                                          // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║                 ...               ║
// ╠     model name (32)               ╣ // name characters
// ║                 ...               ║
// ╠═══════════════════════════════════╣
// ║     mesh count (4)                ║ // # meshes that make up model
// ╠═══════════════════════════════════╣    (no anim)
// ║                 ...               ║
// ╠     mesh ids (mesh-count * 4)     ╣ // ids to mesh files
// ║                 ...               ║
// ╚═══════════════════════════════════╝
template <>
void ResourceLoader::loadFromMetadata<Model>(ResourceMetadata metadata, Model& model){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        SprLog::warn("[ResourceLoader] Model not found");
    }

    uint32 meshCount;

    // read name (ignore)
    f.ignore(32);

    // read mesh count
    f.read((char*)&meshCount, sizeof(uint32));

    // read mesh ids
    std::vector<uint32> meshIds(meshCount);
    for (uint32 i = 0; i < meshCount; i++){
        f.read((char*)&meshIds[i], sizeof(uint32));
    }

    // close file
    f.close();

    // ---- create Model ----
    // instance data
    model.id = m_instanceId++;
    model.resourceId = metadata.resourceId;
    // resource data
    model.meshCount = meshCount;
    model.meshIds = meshIds;
}


// ------------------------------------------------------------------------- //
//    Mesh - .smsh                                                           // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     material id (4)               ║ // material file of mesh
// ╠═══════════════════════════════════╣ 
// ║     index buffer id (4)           ║ // index buffer file
// ╠═══════════════════════════════════╣
// ║     position buffer id (4)        ║ // position buffer file
// ╠═══════════════════════════════════╣ 
// ║     attribute buffer id (4)       ║ // attributes buffer file
// ╚═══════════════════════════════════╝
template <>
void ResourceLoader::loadFromMetadata<Mesh>(ResourceMetadata metadata, Mesh& mesh){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        SprLog::warn("[ResourceLoader] Mesh not found");
    }

    uint32 materialId;
    uint32 indexBufferId;
    uint32 positionBufferId;
    uint32 attributesBufferId;

    // read material id
    f.read((char*)&materialId, sizeof(uint32));

    // read index buffer id
    f.read((char*)&indexBufferId, sizeof(uint32));

    // read position buffer id
    f.read((char*)&positionBufferId, sizeof(uint32));

    // read normal buffer id
    f.read((char*)&attributesBufferId, sizeof(uint32));

    // close file
    f.close();

    // ---- create Mesh ----
    // instance data
    mesh.id = m_instanceId++;
    mesh.resourceId = metadata.resourceId;
    // resource data
    mesh.materialId = materialId;
    mesh.indexBufferId = indexBufferId;
    mesh.positionBufferId = positionBufferId;
    mesh.attributesBufferId = attributesBufferId;
}


// ------------------------------------------------------------------------- //
//    Material - .smtl                                                       // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     base color flag (4)           ║ // base color tex
// ╠───────────────────────────────────╣ 
// ║        - tex id (4)               ║
// ╠───────────────────────────────────╣
// ║                                   ║ 
// ╠                                   ╣
// ║                                   ║ 
// ╠        - base color factor (4*4)  ╣ 
// ║                                   ║ 
// ╠                                   ╣
// ║                                   ║ 
// ╠═══════════════════════════════════╣
// ║     metalroughness flag (4)       ║ // metallic roughness tex
// ╠───────────────────────────────────╣ 
// ║        - tex id (4)               ║ 
// ╠───────────────────────────────────╣
// ║        - metal factor (4)         ║ 
// ╠───────────────────────────────────╣
// ║        - roughness factor (4)     ║ 
// ╠═══════════════════════════════════╣ 
// ║     normal flag (4)               ║ // normal tex
// ╠───────────────────────────────────╣ 
// ║        - tex id (4)               ║
// ╠───────────────────────────────────╣
// ║        - normal scale (4)         ║ 
// ╠═══════════════════════════════════╣
// ║     occlusion flag (4)            ║ // occlusion tex
// ╠───────────────────────────────────╣ 
// ║        - tex id (4)               ║ 
// ╠───────────────────────────────────╣
// ║        - occlusion strength (4)   ║ 
// ╠═══════════════════════════════════╣
// ║     emissive flag (4)             ║ // emissive tex
// ╠───────────────────────────────────╣ 
// ║        - tex id (4)               ║
// ╠───────────────────────────────────╣
// ║                                   ║ 
// ╠                                   ╣
// ║        - emissive factor (3*4)    ║ 
// ╠                                   ╣ 
// ║                                   ║ 
// ╠═══════════════════════════════════╣
// ║     alpha flag (4)                ║ // alpha transparency
// ╠───────────────────────────────────╣ 
// ║        - alpha type (4)           ║ 
// ╠───────────────────────────────────╣
// ║        - alpha cutoff (4)         ║ 
// ╠═══════════════════════════════════╣
// ║     doublesided (4)               ║ // double sided
// ╠═══════════════════════════════════╣ 
// ║     sentinel (4)                  ║ // terminate
// ╚═══════════════════════════════════╝
template <>
void ResourceLoader::loadFromMetadata<Material>(ResourceMetadata metadata, Material& material){
    uint32 materialFlags = 0;

    uint32 baseColorTexId = 0;
    glm::vec4 baseColorFactor;

    uint32 metalRoughTexId = 0;
    float metalFactor;
    float roughnessFactor;

    uint32 normalTexId = 0;
    float normalScale;

    uint32 occlusionTexId = 0;
    float occlusionStrength;

    uint32 emissiveTexId = 0;
    glm::vec3 emissiveFactor;

    uint32 alphaType;
    float alphaCutoff;

    bool doubleSided;
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        SprLog::warn("[ResourceLoader] Material not found");
    }
    bool keepParsing = true;
    while(!f.eof()){
        if (!keepParsing)
            break;
        uint32 materialType = 0;
        if(!f.read((char*)&materialType, sizeof(uint32)))
            break;
        if (f.eof())
            continue;
        switch(materialType) {
            case 1 : // base color
                materialFlags |= 0b1;
                f.read((char*)&baseColorTexId, sizeof(uint32));
                f.read((char*)&baseColorFactor.x, sizeof(float));
                f.read((char*)&baseColorFactor.y, sizeof(float));
                f.read((char*)&baseColorFactor.z, sizeof(float));
                f.read((char*)&baseColorFactor.w, sizeof(float));
                break;
            case 2 : // metalroughness
                materialFlags |= (0b1<<1);
                f.read((char*)&metalRoughTexId, sizeof(uint32));
                f.read((char*)&metalFactor, sizeof(float));
                f.read((char*)&roughnessFactor, sizeof(float));
                break;
            case 3 : // normal
                materialFlags |= (0b1<<2);
                f.read((char*)&normalTexId, sizeof(uint32));
                f.read((char*)&normalScale, sizeof(float));
                break;
            case 4 : // occlusion
                materialFlags |= (0b1<<3);
                f.read((char*)&occlusionTexId, sizeof(uint32));
                f.read((char*)&occlusionStrength, sizeof(float));
                break;
            case 5 : // emissive
                materialFlags |= (0b1<<4);
                f.read((char*)&emissiveTexId, sizeof(uint32));
                f.read((char*)&emissiveFactor.x, sizeof(float));
                f.read((char*)&emissiveFactor.y, sizeof(float));
                f.read((char*)&emissiveFactor.z, sizeof(float));
                break;
            case 6 : // alpha
                materialFlags |= (0b1<<5);
                f.read((char*)&alphaType, sizeof(uint32));
                f.read((char*)&alphaCutoff, sizeof(float));
                break;
            case 7 : // double-sided
                materialFlags |= (0b1<<6);
                doubleSided = true;
                break;
            case 0 : 
                keepParsing = false;
                break;
            default: // unknown
                break;
        }
        if (doubleSided && ((materialFlags | (0b1<<6)) == (0b1<<6))){
            break;
        }
    }
    // close file
    f.close();

    // ---- create Material ----
    // instance data
    material.id = m_instanceId++;
    material.resourceId = metadata.resourceId;
    // resource data
    material.materialFlags = materialFlags;
    if (materialFlags & 0b1){ // base color
        material.baseColorTexId = baseColorTexId;
        material.baseColorFactor = baseColorFactor;
    }
    if (materialFlags & (0b1<<1)){ // metallicroughness
        material.metalRoughTexId = metalRoughTexId;
        material.metalFactor = metalFactor;
    }
    if (materialFlags & (0b1<<2)){ // normal
        material.normalTexId = normalTexId;
        material.normalScale = normalScale;
    }
    if (materialFlags & (0b1<<3)){ // occlusion
        material.occlusionTexId = occlusionTexId;
        material.occlusionStrength = occlusionStrength;
    }
    if (materialFlags & (0b1<<4)){ // emissive
        material.emissiveTexId = emissiveTexId;
        material.emissiveFactor = emissiveFactor;
    }
    if (materialFlags & (0b1<<5)){ // alpha
        material.alphaType = alphaType;
        material.alphaCutoff = alphaCutoff;
    }
    if (materialFlags & (0b1<<6)){ // double-sided
        material.doubleSided = true;
    }
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     buffer id (4)                 ║ // buffer holding tex data
// ╠═══════════════════════════════════╣
// ║     height (4)                    ║ // texture height
// ╠═══════════════════════════════════╣
// ║     width  (4)                    ║ // texture width
// ╠═══════════════════════════════════╣
// ║     components (4)                ║ // 1 - grey | 2 - grey,red | 3 - rgb | 4 - rgba
// ╚═══════════════════════════════════╝
template <>
void ResourceLoader::loadFromMetadata<Texture>(ResourceMetadata metadata, Texture& texture){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        SprLog::warn("[ResourceLoader] Texture not found");
    }

    uint32 bufferId;
    uint32 height;
    uint32 width;
    uint32 components;

    // read buffer id
    f.read((char*)&bufferId, sizeof(uint32));

    // read image height
    f.read((char*)&height, sizeof(uint32));

    // read image width
    f.read((char*)&width, sizeof(uint32));

    // read image type
    f.read((char*)&components, sizeof(uint32));

    // close file
    f.close();

    // ---- create Texture ----
    // instance data
    texture.id = m_instanceId++;
    texture.resourceId = metadata.resourceId;
    // resource data
    texture.bufferId = bufferId;
    texture.height = height;
    texture.width = width;
    texture.components = components;
}


// ------------------------------------------------------------------------- //
//    Buffer - .sbuf                                                         // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     association (4)               ║ // parent type ('smtl', 'stex',..)
// ╠═══════════════════════════════════╣
// ║     element type (4)              ║ // element type (vec3,...)
// ╠═══════════════════════════════════╣
// ║     component type (4)            ║ // component type (f,i,s,...)
// ╠═══════════════════════════════════╣
// ║     byte length (4)               ║ // size of data
// ╠═══════════════════════════════════╣ 
// ║                 ...               ║
// ╠     data (byte-length)            ╣ // stored data
// ║                 ...               ║
// ╚═══════════════════════════════════╝
template <>
void ResourceLoader::loadFromMetadata<Buffer>(ResourceMetadata metadata, Buffer& buffer){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        SprLog::warn("[ResourceLoader] Buffer not found, id: " + std::to_string(metadata.resourceId) + ", name: " + metadata.name);
    }

    unsigned char association[4];
    uint32 elementType;
    uint32 componentType;
    uint32 byteLength;

    // read association
    f.read((char*)&association, sizeof(uint32));

    // read element type
    f.read((char*)&elementType, sizeof(uint32));

    // read component type
    f.read((char*)&componentType, sizeof(uint32));
    
    // read byte length
    f.read((char*)&byteLength, sizeof(uint32));

    // read data
    buffer.data.resize(byteLength);
    f.read((char*)buffer.data.data(), byteLength);   

    // close file
    f.close();

    // ---- create Buffer ----
    // instance data
    buffer.id = m_instanceId++;
    buffer.resourceId = metadata.resourceId;
    // resource data
    buffer.elementType = elementType;
    buffer.componentType = componentType;
    buffer.byteLength = byteLength;
}


// ----------------------------------------------------------------------------
//    Audio - .***
//    iw
template <>
void ResourceLoader::loadFromMetadata<Audio>(ResourceMetadata metadata, Audio& audio){
}


// ----------------------------------------------------------------------------
//    Shader - .glsl
//    iw
template <>
void ResourceLoader::loadFromMetadata<Shader>(ResourceMetadata metadata, Shader& shader){
}

}