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
int AssetRegisterer::loadBuffer(std::string path, ResourceMetadata& modelData, mio::mmap_source& file, ModelHeader& model, uint32 offset, uint32 length){
    int totalBytes = 0;
    ResourceMetadata metadata = {
        .name = modelData.name+"_"+std::to_string(m_id),
        .resourceType = SPR_BUFFER,
        .resourceId = m_id++,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = offset,
        .byteLength = length,
        .index = 0
    };

    totalBytes += length;

    metadata.sizeTotal = totalBytes;
    m_metadataMap[metadata.name] = metadata;
    return totalBytes;
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
int AssetRegisterer::loadTexture(std::string path, ResourceMetadata& modelData, bool subresource, mio::mmap_source& file, ModelHeader& model, uint32 index){
    int totalBytes = 0;
    uint32 offset = subresource ? model.textureBufferOffset : 0;
    TextureLayout& texture = ((TextureLayout*)(file.data() + offset))[index];
    ResourceMetadata metadata = {
        .name = modelData.name,
        .resourceType = SPR_TEXTURE,
        .resourceId = m_id++,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = offset,
        .byteLength = 0,
        .index = index
    };
    
    if (subresource){
        BlobHeader& blob = ((BlobHeader*)(file.data() + model.blobHeaderOffset))[0];
        totalBytes += loadBuffer(path, modelData, file, model, blob.textureRegionOffset + texture.dataOffset, texture.dataSizeBytes);
        metadata.sizeTotal = totalBytes;
        m_metadataMap[metadata.name+"_"+std::to_string(m_id)] = metadata;
    } else {
        totalBytes += loadBuffer(path, modelData, file, model, texture.dataOffset, texture.dataSizeBytes);
        metadata.sizeTotal = totalBytes;
        m_nonSubresourceTextureMap[metadata.name] = metadata;
    }

    return totalBytes;
}


// ------------------------------------------------------------------------- //
//    Material - .smtl                                                       // 
// ------------------------------------------------------------------------- //
int AssetRegisterer::loadMaterial(std::string path, ResourceMetadata& modelData, mio::mmap_source& file, ModelHeader& model, MeshLayout& mesh){
    int totalBytes = 0;
    MaterialLayout& material = ((MaterialLayout*)(file.data() + model.materialBufferOffset))[mesh.materialIndex];
    ResourceMetadata metadata = {
        .name = modelData.name+"_"+std::to_string(m_id),
        .resourceType = SPR_MATERIAL,
        .resourceId = m_id++,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = model.materialBufferOffset,
        .byteLength = 0,
        .index = mesh.materialIndex
    };    
    
    if (material.materialFlags & 0b1 && (m_texturePresenceMap.count(material.bc_textureIndex) == 0)){ // base color
        totalBytes += loadTexture(path, modelData, true, file, model, material.bc_textureIndex);
        m_texturePresenceMap[material.bc_textureIndex] = material.bc_textureIndex;
    }
    if (material.materialFlags & (0b1<<1) && (m_texturePresenceMap.count(material.mr_textureIndex) == 0)){ // metallicroughness
        totalBytes += loadTexture(path, modelData, true, file, model, material.mr_textureIndex);
        m_texturePresenceMap[material.mr_textureIndex] = material.mr_textureIndex;
    }
    if (material.materialFlags & (0b1<<2) && (m_texturePresenceMap.count(material.n_textureIndex) == 0)){ // normal
        totalBytes += loadTexture(path, modelData, true, file, model, material.n_textureIndex);
        m_texturePresenceMap[material.n_textureIndex] = material.n_textureIndex;
    }
    if (material.materialFlags & (0b1<<3) && (m_texturePresenceMap.count(material.o_textureIndex) == 0)){ // occlusion
        totalBytes += loadTexture(path, modelData, true, file, model, material.o_textureIndex);
        m_texturePresenceMap[material.o_textureIndex] = material.o_textureIndex;
    }
    if (material.materialFlags & (0b1<<4) && (m_texturePresenceMap.count(material.e_textureIndex) == 0)){ // emissive
        totalBytes += loadTexture(path, modelData, true, file, model, material.e_textureIndex);
        m_texturePresenceMap[material.e_textureIndex] = material.e_textureIndex;
    }

    metadata.sizeTotal = totalBytes;
    m_metadataMap[metadata.name] = metadata;
    return totalBytes;
}


// ------------------------------------------------------------------------- //
//    Mesh - .smsh                                                           // 
// ------------------------------------------------------------------------- //
int AssetRegisterer::loadMesh(std::string path, ResourceMetadata& modelData, mio::mmap_source& file, ModelHeader& model, uint32 index){
    int totalBytes = 0;
    
    MeshLayout& mesh = ((MeshLayout*)(file.data() + model.meshBufferOffset))[index];
    ResourceMetadata metadata = {
        .name = modelData.name+"_"+std::to_string(m_id),
        .resourceType = SPR_MESH,
        .resourceId = m_id++,
        .parentId = modelData.resourceId,
        .sizeTotal = 0,
        .byteOffset = model.meshBufferOffset,
        .byteLength = 0,
        .index = index
    };

    BlobHeader& blob = ((BlobHeader*)(file.data() + model.blobHeaderOffset))[0];
    
    // material
    totalBytes += loadMaterial(path, modelData, file, model, mesh);

    // index
    if (mesh.indexDataSizeBytes > 0)
        totalBytes += loadBuffer(path, modelData, file, model, blob.indexRegionOffset + mesh.indexDataOffset, mesh.indexDataSizeBytes);

    // position
    if (mesh.positionDataSizeBytes > 0)
        totalBytes += loadBuffer(path, modelData, file, model, blob.positionRegionOffset + mesh.positionDataOffset, mesh.positionDataSizeBytes);

    // attributes
    if (mesh.attributeDataSizeBytes > 0)
        totalBytes += loadBuffer(path, modelData, file, model, blob.attributeRegionOffset + mesh.attributeDataOffset, mesh.attributeDataSizeBytes);

    metadata.sizeTotal = totalBytes;
    m_metadataMap[metadata.name] = metadata;
    return totalBytes;
}


// ------------------------------------------------------------------------- //
//    Model - .smdl                                                          // 
// ------------------------------------------------------------------------- //
int AssetRegisterer::loadModel(std::string path, mio::mmap_source& file){
    int totalBytes = 0;
    
    ModelHeader& model = ((ModelHeader*)(file.data() + 0))[0];
    ResourceMetadata metadata = {
        .name = model.name,
        .resourceType = SPR_MODEL,
        .resourceId = m_id,
        .parentId = m_id++,
        .sizeTotal = 0,
        .byteOffset = 0,
        .byteLength = 0
    };
    
    for (int i = 0; i < model.meshCount; i++){
        totalBytes += loadMesh(path, metadata, file, model, i);
    }
    metadata.sizeTotal = totalBytes;
    m_modelMetadataMap[metadata.name] = metadata;
    return totalBytes;
}

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

    // // subresource info
    // manifest["subresourceCount"] = m_metadataMap.size();
    // manifest["subresources"] = nlohmann::json::array();
    // // write subresources
    // for (std::pair<std::string, ResourceMetadata> metadata : m_metadataMap){
    //     std::string name = metadata.first;
    //     int id = metadata.second.resourceId;
    //     nlohmann::json subresource;
    //     subresource["id"] = id;
    //     subresource["parentId"] = metadata.second.parentId;
    //     subresource["name"] = name;
    //     subresource["sizeTotal"] = metadata.second.sizeTotal;
    //     subresource["type"] = ResourceTypes::typeToString(metadata.second.resourceType);
    //     subresource["byteOffset"] = metadata.second.byteOffset;
    //     subresource["byteLength"] = metadata.second.byteLength;
    //     subresource["index"] = metadata.second.index;
    //     manifest["subresources"].push_back(subresource);
    // }

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

void AssetRegisterer::registerDirectory(std::string dir){
    int totalSizeBytes = 0;
    
    std::error_code error;
    mio::mmap_source ro_mmap;
    
    // process models and subresources
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "assets/")){
        std::string ext = dirEntry.path().extension();

        if (ext != ".smdl")
            continue;
        
        ro_mmap.map(dirEntry.path().string(), error);
        m_texturePresenceMap.clear();
        totalSizeBytes += loadModel(dirEntry.path().string(), ro_mmap);
        ro_mmap.unmap();
    }

    // process non-subresource textures
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "assets/")){
        std::string name = std::filesystem::path(dirEntry).stem();
        std::string ext = dirEntry.path().extension();

        if (ext != ".stex")
            continue;
        
        ro_mmap.map(dirEntry.path().string(), error);
        ModelHeader& model = ((ModelHeader*)(ro_mmap.data() + 0))[0];
        ResourceMetadata metadata = {
            .name = name,
            .resourceType = SPR_MODEL,
            .resourceId = ++m_id,
            .parentId = m_id,
            .sizeTotal = 0,
            .byteOffset = sizeof(TextureLayout),
            .byteLength = 0
        };
        totalSizeBytes += loadTexture(dirEntry.path().string(), metadata, false, ro_mmap, model, 0);
        ro_mmap.unmap();
    }

    // write asset_ids.h
    writeHeader();

    // write asset_manifest.h
    writeManifest(totalSizeBytes);
}

}