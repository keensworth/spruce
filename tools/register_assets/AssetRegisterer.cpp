#include "AssetRegisterer.h"
#include "json.hpp"
#include "debug/SprLog.h"
#include "../../external/mio/mio.h"
#include "../gltf/Resources.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>

namespace spr::tools{


// ------------------------------------------------------------------------- //
//    Buffer - .sbuf                                                         // 
// ------------------------------------------------------------------------- //
LoadResult AssetRegisterer::loadBuffer(ResourceMetadata& modelData, uint32 offset, uint32 length){
    uint32 id = m_id++;

    ResourceMetadata metadata = {
        .name = {modelData.name+"_"+std::to_string(id)},
        .resourceType = SPR_BUFFER,
        .resourceId = id,
        .parentId = modelData.resourceId,
        .sizeTotal = length,
        .byteOffset = offset,
        .byteLength = length,
        .index = 0
    };

    m_metadataMap[metadata.name] = metadata;
    return {length, id};
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
LoadResult AssetRegisterer::loadTexture(ResourceMetadata& modelData, uint32 index){
    uint32 id = m_id++;

    ModelHeader& model = ((ModelHeader*)rw_mmap.data())[0];
    TextureLayout* textures = (TextureLayout*)(rw_mmap.data() + model.textureBufferOffset);
    TextureLayout& texture = textures[index];
    texture.id = id;

    ResourceMetadata metadata = {
        .name = {modelData.name},
        .resourceType = SPR_TEXTURE,
        .resourceId = id,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = model.textureBufferOffset,
        .byteLength = 0,
        .index = index
    };
    
    BlobHeader& blob = ((BlobHeader*)(rw_mmap.data() + model.blobHeaderOffset))[0];
    LoadResult result = loadBuffer(modelData, blob.textureRegionOffset + texture.dataOffset, texture.dataSizeBytes);
    metadata.sizeTotal = result.sizeBytes;
    texture.dataBufferId = result.resultId;

    m_metadataMap[{metadata.name+"_"+std::to_string(id)}] = metadata;
    return {result.sizeBytes, id};
}

LoadResult AssetRegisterer::loadDedicatedTexture(ResourceMetadata& modelData){
    TextureLayout* textures = (TextureLayout*)(rw_mmap.data());
    TextureLayout& texture = textures[0];

    ResourceMetadata& metadata = m_nonSubresourceTextureMap[modelData.name];
    
    LoadResult result = loadBuffer(metadata, sizeof(TextureLayout), texture.dataSizeBytes);
    metadata.sizeTotal = result.sizeBytes;
    texture.dataBufferId = result.resultId;

    return {result.sizeBytes, metadata.resourceId};
}

void AssetRegisterer::checkinDedicatedTexture(ResourceMetadata& modelData){
    uint32 id = m_id++;

    TextureLayout* textures = (TextureLayout*)(rw_mmap.data());
    TextureLayout& texture = textures[0];
    texture.id = id;

    ResourceMetadata metadata = {
        .name = {modelData.name},
        .resourceType = SPR_TEXTURE,
        .resourceId = id,
        .parentId = id,
        .sizeTotal = 0,
        .byteOffset = sizeof(TextureLayout),
        .byteLength = 0,
        .index = 0
    };

    m_nonSubresourceTextureMap[metadata.name] = metadata;
}


// ------------------------------------------------------------------------- //
//    Material - .smtl                                                       // 
// ------------------------------------------------------------------------- //
LoadResult AssetRegisterer::loadMaterial(ResourceMetadata& modelData, MeshLayout& mesh){
    

    ModelHeader& model = ((ModelHeader*)rw_mmap.data())[0];
    MaterialLayout* materials = (MaterialLayout*)(rw_mmap.data() + model.materialBufferOffset);
    MaterialLayout& material = materials[mesh.materialIndex];

    if (m_materialPresenceMap.count(mesh.materialIndex)){
        return {0, m_materialPresenceMap[mesh.materialIndex]};
    }
    
    uint32 id = m_id++;
    material.id = id;

    ResourceMetadata metadata = {
        .name = {modelData.name+"_"+std::to_string(id)},
        .resourceType = SPR_MATERIAL,
        .resourceId = id,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = model.materialBufferOffset,
        .byteLength = 0,
        .index = mesh.materialIndex
    };
    
    uint32 totalBytes = 0;
    LoadResult result;
    if (material.materialFlags & 0b1){ // base color
        result = loadTexture(modelData, material.bc_textureIndex);
        totalBytes += result.sizeBytes;
        m_texturePresenceMap[material.bc_textureIndex] = material.bc_textureIndex;
    }
    
    if (material.materialFlags & (0b1<<1)){ // metallicroughness
        result = loadTexture(modelData, material.mr_textureIndex);
        totalBytes += result.sizeBytes;
        m_texturePresenceMap[material.mr_textureIndex] = material.mr_textureIndex;
    }
    
    if (material.materialFlags & (0b1<<2)){ // normal
        result = loadTexture(modelData, material.n_textureIndex);
        totalBytes += result.sizeBytes;
        m_texturePresenceMap[material.n_textureIndex] = material.n_textureIndex;
    }
    
    if (material.materialFlags & (0b1<<3)){ // occlusion
        result = loadTexture(modelData, material.o_textureIndex);
        totalBytes += result.sizeBytes;
        m_texturePresenceMap[material.o_textureIndex] = material.o_textureIndex;
    }
    
    if (material.materialFlags & (0b1<<4)){ // emissive
        result = loadTexture(modelData, material.e_textureIndex);
        totalBytes += result.sizeBytes;
        m_texturePresenceMap[material.e_textureIndex] = material.e_textureIndex;
    }
    
    metadata.sizeTotal = totalBytes;
    m_metadataMap[metadata.name] = metadata;
    m_materialPresenceMap[mesh.materialIndex] = id;
    return {totalBytes, id};
}


// ------------------------------------------------------------------------- //
//    Mesh - .smsh                                                           // 
// ------------------------------------------------------------------------- //
LoadResult AssetRegisterer::loadMesh(ResourceMetadata& modelData, uint32 index){
    uint32 id = m_id++;

    ModelHeader& model = ((ModelHeader*)rw_mmap.data())[0];
    MeshLayout* meshes = (MeshLayout*)(rw_mmap.data() + model.meshBufferOffset);
    MeshLayout& mesh = meshes[index];
    mesh.id = id;

    ResourceMetadata metadata = {
        .name = {modelData.name+"_"+std::to_string(id)},
        .resourceType = SPR_MESH,
        .resourceId = id,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = model.meshBufferOffset,
        .byteLength = 0,
        .index = index
    };

    BlobHeader& blob = ((BlobHeader*)(rw_mmap.data() + model.blobHeaderOffset))[0];
    
    // material
    uint32 totalBytes = 0;
    LoadResult result = loadMaterial(modelData, mesh);
    totalBytes += result.sizeBytes;

    // index
    if (mesh.indexDataSizeBytes > 0){
        result = loadBuffer(modelData, blob.indexRegionOffset + mesh.indexDataOffset, mesh.indexDataSizeBytes);
        totalBytes += result.sizeBytes;
        mesh.indexBufferId = result.resultId;
    }

    // position
    if (mesh.positionDataSizeBytes > 0){
        result = loadBuffer(modelData, blob.positionRegionOffset + mesh.positionDataOffset, mesh.positionDataSizeBytes);
        totalBytes += result.sizeBytes;
        mesh.positionBufferId = result.resultId;
    }

    // attributes
    if (mesh.attributeDataSizeBytes > 0){
        result = loadBuffer(modelData, blob.attributeRegionOffset + mesh.attributeDataOffset, mesh.attributeDataSizeBytes);
        totalBytes += result.sizeBytes;
        mesh.attributeBufferId = result.resultId;
    }

    metadata.sizeTotal = totalBytes;
    m_metadataMap[metadata.name] = metadata;
    return {totalBytes, id};
}


// ------------------------------------------------------------------------- //
//    Model - .smdl                                                          // 
// ------------------------------------------------------------------------- //
LoadResult AssetRegisterer::loadModel(){
    ModelHeader& model = ((ModelHeader*)rw_mmap.data())[0];
    ResourceMetadata& metadata = m_modelMetadataMap[model.name];
    
    uint32 totalBytes = 0;
    LoadResult result;
    for (int i = 0; i < model.meshCount; i++){            
        result = loadMesh(metadata, i);
        totalBytes += result.sizeBytes;
    }
    metadata.sizeTotal = totalBytes;
    return {totalBytes, model.id};
}

void AssetRegisterer::checkinModel(){
    uint32 id = m_id++;

    ModelHeader& model = ((ModelHeader*)rw_mmap.data())[0];
    model.id = id;

    ResourceMetadata metadata = {
        .name = {model.name},
        .resourceType = SPR_MODEL,
        .resourceId = id,
        .parentId = id,
        .sizeTotal = 0,
        .byteOffset = 0,
        .byteLength = 0
    };

    m_modelMetadataMap[metadata.name] = metadata;
}


// ------------------------------------------------------------------------- //
//    AssetRegisterer                                                        // 
// ------------------------------------------------------------------------- //
void AssetRegisterer::registerDirectory(std::string dir){
    uint32 totalSizeBytes = 0;
    LoadResult result = {0 , 0};
    for (int i = 0; i < 2; i++){
        // process models and subresources
        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "assets/")){
            std::string path = dirEntry.path().string();
            std::string ext = dirEntry.path().extension();

            if (ext != ".smdl")
                continue;

            rw_mmap = mio::make_mmap_sink(path, 0, mio::map_entire_file, m_error);
            m_texturePresenceMap.clear();
            m_materialPresenceMap.clear();

            if (i == 0){
                checkinModel();
            } else { 
                result = loadModel();
                totalSizeBytes += result.sizeBytes;
            }
            rw_mmap.sync(m_error);
            rw_mmap.unmap();
        }

        // process non-subresource textures
        for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "assets/")){
            std::string path = dirEntry.path().string();
            std::string name = std::filesystem::path(dirEntry).stem();
            std::string ext = dirEntry.path().extension();

            if (ext != ".stex")
                continue;
            
            rw_mmap = mio::make_mmap_sink(path, 0, mio::map_entire_file, m_error);
            m_texturePresenceMap.clear();
            m_materialPresenceMap.clear();

            ResourceMetadata metadata = {
                .name = {name},
                .resourceType = SPR_MODEL,
                .resourceId = 0,
                .parentId = 0,
                .sizeTotal = 0,
                .byteOffset = sizeof(TextureLayout),
                .byteLength = 0
            };
            if (i == 0){
                checkinDedicatedTexture(metadata);
            } else {
                result = loadDedicatedTexture(metadata);
                totalSizeBytes += result.sizeBytes;
            }
            rw_mmap.sync(m_error);
            rw_mmap.unmap();
        }
    }

    // write asset_ids.h
    writeHeader();

    // write asset_manifest.h
    writeManifest(totalSizeBytes);
}


// ------------------------------------------------------------------------- //
//    .h / .json                                                             // 
// ------------------------------------------------------------------------- //
void AssetRegisterer::writeHeader(){
    std::ofstream f;
    f.open ("../data/asset_ids.h");
    if (!f.is_open()){
        return;
    }
    // write contents
    f << "#pragma once\n";
    f << "\n";
    f << "#include \"../external/flat_hash_map/flat_hash_map.hpp\"\n";
    f << "\n";
    f << "\n";
    f << "namespace spr::data{\n";
    f << "typedef enum {\n";
    f << "    // === Null Resource ===\n";
    f << "    null_resource = 0,\n";
    f << "\n";
    f << "    // === Imported Resources ===\n";
    f << "    // models:\n";
    uint32 conflictId = 1;
    for (std::pair<std::string, ResourceMetadata> metadata : m_modelMetadataMap){
        std::string name = metadata.first;
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c){ return std::tolower(c); }); // lowercase
        std::replace(name.begin(), name.end(), ' ', '_');    // " " -> "_"

        m_headerLowerNameMap[name]++;
        uint32 nameCount = m_headerLowerNameMap.at(name);

        std::string suffix = nameCount > 1 ? std::to_string(conflictId) : "";
        f << "    " << name << suffix << " = " << std::to_string(metadata.second.resourceId) << ",\n";
    }
    f << "\n";
    f << "    // textures:\n";
    for (std::pair<std::string, ResourceMetadata> metadata : m_nonSubresourceTextureMap){
        std::string name = metadata.first;
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c){ return std::tolower(c); }); // lowercase
        std::replace(name.begin(), name.end(), ' ', '_');    // " " -> "_"

        m_headerLowerNameMap[name]++;
        uint32 nameCount = m_headerLowerNameMap.at(name);

        std::string suffix = nameCount > 1 ? std::to_string(conflictId) : "";
        f << "    " << name << suffix << " = " << std::to_string(metadata.second.resourceId) << ",\n";
    }
    f << "\n";
    f << "    // shaders:\n";
    f << "    // audio:\n";
    f << "} ResourceId;\n";
    f << "\n";
    f << "class ResourceIds{\n";
    f << "public:\n";
    f << "    // Filename <-> ResourceId map\n";
    f << "    ska::flat_hash_map<std::string, uint32_t> idMap = \n";
    f << "    {\n";
    f << "        // models\n";
    for (std::pair<std::string, ResourceMetadata> metadata : m_modelMetadataMap){
        std::string name = metadata.first;
        f << "        {\"" << name << "\", " << metadata.second.resourceId << "},\n";
    }
    f << "        // textures\n";
    for (std::pair<std::string, ResourceMetadata> metadata : m_nonSubresourceTextureMap){
        std::string name = metadata.first;
        f << "        {\"" << name << "\", " << metadata.second.resourceId << "},\n";
    }
    // f << "        // sub-resources\n";
    // for (std::pair<std::string, ResourceMetadata> metadata : m_metadataMap){
    //     std::string name = metadata.first;
    //     f << "        {\"" << name << "\", " << metadata.second.resourceId << "},\n";
    // }
    f << "    };\n";
    f << "\n";
    f << "    uint32_t getIdFromName(std::string name){\n";
    f << "        return idMap[name];\n";
    f << "    }\n";
    f << "};\n";
    f << "}\n";

    // close file
    f.close();
}

void AssetRegisterer::writeManifest(int totalBytes){
    nlohmann::json manifest;

    // get date/time
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%m-%d-%Y %H:%M:%S");
    auto str = oss.str();
    
    // global info
    manifest["date"] = str;
    manifest["sizeBytes"] = totalBytes;

    // model info
    manifest["modelCount"] = m_modelMetadataMap.size();
    manifest["models"] = nlohmann::json::array();
    // write models
    for (std::pair<std::string, ResourceMetadata> metadata : m_modelMetadataMap){
        std::string name = metadata.first;
        int id = metadata.second.resourceId;
        nlohmann::json model;
        model["id"] = id;
        model["parentId"] = metadata.second.parentId;
        model["name"] = name;
        model["sizeTotal"] = metadata.second.sizeTotal;
        model["type"] = typeToString(metadata.second.resourceType);
        manifest["models"].push_back(model);
    }

    // nonsubresouce texture info
    manifest["nonSubresourceTextureCount"] = m_nonSubresourceTextureMap.size();
    manifest["nonSubresourceTextures"] = nlohmann::json::array();
    // write models
    for (std::pair<std::string, ResourceMetadata> metadata : m_nonSubresourceTextureMap){
        std::string name = metadata.first;
        int id = metadata.second.resourceId;
        nlohmann::json model;
        model["id"] = id;
        model["parentId"] = metadata.second.parentId;
        model["name"] = name;
        model["sizeTotal"] = metadata.second.sizeTotal;
        model["type"] = typeToString(metadata.second.resourceType);
        manifest["nonSubresourceTextures"].push_back(model);
    }

    // write JSON to file
    std::ofstream f;
    f.open ("../data/asset_manifest.json");
    if (!f.is_open()){
        return;
    }

    // output to file
    f << std::setw(4) << manifest << std::endl;

    // close
    f.close();
}
}