#include "AssetRegisterer.h"
#include "json.hpp"

namespace spr::tools{


// ------------------------------------------------------------------------- //
//    Buffer - .sbuf                                                         // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     element type (4)              ║ // element type (f,i,s,...)
// ╠═══════════════════════════════════╣
// ║     component type (4)            ║ // component type (vec3,...)
// ╠═══════════════════════════════════╣
// ║     byte length (4)               ║ // size of data
// ╠═══════════════════════════════════╣ 
// ║                 ...               ║
// ╠     data (byte-length)            ╣ // stored data
// ║                 ...               ║
// ╚═══════════════════════════════════╝
int AssetRegisterer::loadBuffer(std::string path){
    std::cout << "    Registering buffer:   " << path << std::endl;
    // open file
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to load." << std::endl;
        return 0;
    }

    uint32 elementType;
    uint32 componentType;
    uint32 byteLength;
    char* data;

    // read element type
    f.read((char*)&elementType, sizeof(uint32));

    // read component type
    f.read((char*)&componentType, sizeof(uint32));
    
    // read byte length
    f.read((char*)&byteLength, sizeof(uint32));

    // read data
    data = (char*)malloc(byteLength*sizeof(char));
    f.read(data, byteLength);
    free(data);

    // close file
    f.close();

    // === Create metadata ===
    std::string name = std::filesystem::path(path).stem();
    size_t found = name.find_last_of("_");

    ResourceMetadata metadata;
    metadata.name = std::string(name);
    metadata.resourceType = SPR_BUFFER;
    metadata.resourceId = m_id++;
    int totalBytes = 0;
    
    totalBytes += byteLength;

    metadata.sizeBytes = totalBytes;
    // store metadata
    m_metadataMap[name] = metadata;

    return totalBytes;
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     buffer id (4)                 ║ // buffer holding tex data
// ╠═══════════════════════════════════╣
// ║     height (4)                    ║ // texture height
// ╠═══════════════════════════════════╣
// ║     width  id (4)                 ║ // texture width
// ╠═══════════════════════════════════╣
// ║     components (4)                ║ // 1 - grey | 2 - grey,red | 3 - rgb | 4 - rgba
// ╚═══════════════════════════════════╝
int AssetRegisterer::loadTexture(std::string path, bool subresource){
    std::cout << "    Registering texture:  " << path << std::endl;
    // open file
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to load." << std::endl;
        return 0;
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

    // === Create metadata ===
    std::string name = std::filesystem::path(path).stem();
    size_t found = name.find_last_of("_");

    ResourceMetadata metadata;
    metadata.name = std::string(name);
    metadata.resourceType = SPR_TEXTURE;
    metadata.resourceId = m_id++;
    int totalBytes = 0;
    
    if(subresource){
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(bufferId) + ResourceTypes::getExtension(SPR_BUFFER)); 
    } else {
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name + "_" + std::to_string(bufferId) + ResourceTypes::getExtension(SPR_BUFFER)); 
    }
    metadata.sizeBytes = totalBytes;
    // store metadata
    if (subresource)
        m_metadataMap[name] = metadata;
    else
        m_nonSubresourceTextureMap[name] = metadata;

    return totalBytes;
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
// ╚═══════════════════════════════════╝ 
int AssetRegisterer::loadMaterial(std::string path){
    std::cout << "    Registering material: " << path << std::endl;
    // open file
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to load." << std::endl;
        return 0;
    }

    uint32 materialFlags;

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

    while(!f.eof()){
        uint32 materialType;
        f.read((char*)&materialType, sizeof(uint32));
        if (f.eof())
            continue;
        switch(materialType) {
            case 1 : // base color
                materialFlags |= 0b1;
                f.read((char*)&baseColorTexId, sizeof(uint32));
                baseColorTexId &= 0xffff;
                f.read((char*)&baseColorFactor.x, sizeof(float));
                f.read((char*)&baseColorFactor.y, sizeof(float));
                f.read((char*)&baseColorFactor.z, sizeof(float));
                f.read((char*)&baseColorFactor.w, sizeof(float));
                break;
            case 2 : // metalroughness
                materialFlags |= (0b1<<1);
                f.read((char*)&metalRoughTexId, sizeof(uint32));
                metalRoughTexId &= 0xffff;
                f.read((char*)&metalFactor, sizeof(float));
                f.read((char*)&roughnessFactor, sizeof(float));
                break;
            case 3 : // normal
                materialFlags |= (0b1<<2);
                f.read((char*)&normalTexId, sizeof(uint32));
                normalTexId &= 0xffff;
                f.read((char*)&normalScale, sizeof(float));
                break;
            case 4 : // occlusion
                materialFlags |= (0b1<<3);
                f.read((char*)&occlusionTexId, sizeof(uint32));
                occlusionTexId &= 0xffff;
                f.read((char*)&occlusionStrength, sizeof(float));
                break;
            case 5 : // emissive
                materialFlags |= (0b1<<4);
                f.read((char*)&emissiveTexId, sizeof(uint32));
                emissiveTexId &= 0xffff;
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
            default: // unknown
                // log error
                break;
        }
    }

    // close file
    f.close();

    // === Create metadata ===
    std::string name = std::filesystem::path(path).stem();
    size_t found = name.find_last_of("_");

    ResourceMetadata metadata;
    metadata.name = std::string(name);
    metadata.resourceType = SPR_MATERIAL;
    metadata.resourceId = m_id++;
    int totalBytes = 0;
    
    if (materialFlags & 0b1 && baseColorTexId > 0){ // base color
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(baseColorTexId) + ResourceTypes::getExtension(SPR_TEXTURE), true);
    }
    if (materialFlags & (0b1<<1) && metalRoughTexId > 0){ // metallicroughness
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(metalRoughTexId) + ResourceTypes::getExtension(SPR_TEXTURE), true);
    }
    if (materialFlags & (0b1<<2) && normalTexId > 0){ // normal
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(normalTexId) + ResourceTypes::getExtension(SPR_TEXTURE), true);
    }
    if (materialFlags & (0b1<<3) && occlusionTexId > 0){ // occlusion
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(occlusionTexId) + ResourceTypes::getExtension(SPR_TEXTURE), true);
    }
    if (materialFlags & (0b1<<4) && emissiveTexId > 0){ // emissive
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(emissiveTexId) + ResourceTypes::getExtension(SPR_TEXTURE), true);
    }

    metadata.sizeBytes = totalBytes;
    // store metadata
    m_metadataMap[name] = metadata;

    return totalBytes;

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
int AssetRegisterer::loadMesh(std::string path){
    std::cout << "    Registering mesh:     " << path << std::endl;
    // open file
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to load." << std::endl;
        return 0;
    }

    uint32 materialId;
    uint32 indexBufferId;
    uint32 positionBufferId;
    uint32 attributesBufferId;

    // read material id
    f.read((char*)&materialId, sizeof(uint32));

    // read index buffer id
    f.read((char*)&indexBufferId, sizeof(uint32));
    if (indexBufferId > 0)
        indexBufferId &= 0xffff;

    // read position buffer id
    f.read((char*)&positionBufferId, sizeof(uint32));
    if (positionBufferId > 0)
        positionBufferId &= 0xffff;

    // read normal buffer id
    f.read((char*)&attributesBufferId, sizeof(uint32));
    if (attributesBufferId > 0)
        attributesBufferId &= 0xffff;

    // close file
    f.close();

    // === Create metadata ===
    std::string name = std::filesystem::path(path).stem();

    ResourceMetadata metadata;
    metadata.name = name;
    metadata.resourceType = SPR_MESH;
    metadata.resourceId = m_id++;
    int totalBytes = 0;
    // material
    size_t found = name.find_last_of("_");
    if (materialId > 0)
        totalBytes += loadMaterial(ResourceTypes::getPath(SPR_MATERIAL) + name.substr(0,found) + "_" + std::to_string(materialId) + ResourceTypes::getExtension(SPR_MATERIAL));
    // index
    if (indexBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(indexBufferId) + ResourceTypes::getExtension(SPR_BUFFER));
    // position
    if (positionBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(positionBufferId) + ResourceTypes::getExtension(SPR_BUFFER));
    // normal
    if (attributesBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(attributesBufferId) + ResourceTypes::getExtension(SPR_BUFFER));  

    metadata.sizeBytes = totalBytes;
    // store metadata
    m_metadataMap[name] = metadata;

    return totalBytes;
}


// ------------------------------------------------------------------------- //
//    Model - .smdl                                                          // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     name length (4)               ║ // # characters in name
// ╠═══════════════════════════════════╣
// ║                 ...               ║
// ╠     name (name-length)            ╣ // name characters
// ║                 ...               ║
// ╠═══════════════════════════════════╣
// ║     mesh count (4)                ║ // # meshes that make up model
// ╠═══════════════════════════════════╣    (no anim)
// ║                 ...               ║
// ╠     mesh ids (mesh-count * 4)     ╣ // ids to mesh files
// ║                 ...               ║
// ╚═══════════════════════════════════╝
int AssetRegisterer::loadModel(std::string path){
    std::cout << "Registering model: " << path << std::endl;
    // open file
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to load." << std::endl;
        return 0;
    }
    std::cout << "  f: opened" << std::endl;

    uint32 nameLength;
    std::string name;
    uint32 meshCount;

    // read name length
    f.read((char*)&nameLength, sizeof(uint32));
    std::cout << "  r: nameLength: " << nameLength << std::endl;

    // read name
    name.resize(nameLength);
    f.read(&name[0], nameLength);
    std::cout << "  r: name: " << name << std::endl;

    // read mesh count
    f.read((char*)&meshCount, sizeof(uint32));
    std::cout << "  r: meshCount: " << meshCount << std::endl;

    // read mesh ids
    std::vector<uint32> meshIds(meshCount);
    for (uint32 i = 0; i < meshCount; i++){
        f.read((char*)&meshIds[i], sizeof(uint32));
        std::cout << "  r: meshId: " << meshIds[i] << std::endl;
    }

    // close file
    f.close();
    std::cout << "  f: closed" << std::endl;

    // === Create metadata ===
    ResourceMetadata metadata;
    metadata.name = name;
    metadata.resourceType = SPR_MODEL;
    metadata.resourceId = m_id++;
    int totalBytes = 0;
    for (uint32 id : meshIds){
        totalBytes += loadMesh(ResourceTypes::getPath(SPR_MESH) + name + "_" + std::to_string(id) + ResourceTypes::getExtension(SPR_MESH));
    }
    metadata.sizeBytes = totalBytes;
    // store metadata
    m_modelMetadataMap[name] = metadata;

    return totalBytes;
}

void AssetRegisterer::writeHeader(){
    std::cout << "Writing asset_ids.h" << std::endl;
    std::ofstream f;
    f.open ("../data/asset_ids.h");
    if (!f.is_open()){
        std::cout << "Failed to create file" << std::endl;
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
    for (std::pair<std::string, ResourceMetadata> metadata : m_modelMetadataMap){
        std::string name = metadata.first;
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c){ return std::tolower(c); }); // lowercase
        std::replace(name.begin(), name.end(), ' ', '_');    // " " -> "_"
        f << "    " << name << " = " << std::to_string(metadata.second.resourceId) << ",\n";
    }
    f << "\n";
    f << "    // textures:\n";
    for (std::pair<std::string, ResourceMetadata> metadata : m_nonSubresourceTextureMap){
        std::string name = metadata.first;
        std::transform(name.begin(), name.end(), name.begin(),
            [](unsigned char c){ return std::tolower(c); }); // lowercase
        std::replace(name.begin(), name.end(), ' ', '_');    // " " -> "_"
        f << "    " << name << " = " << std::to_string(metadata.second.resourceId) << ",\n";
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
        int id = metadata.second.resourceId;
        f << "        {\"" << name << "\", " << metadata.second.resourceId << "},\n";
    }
    f << "        // textures\n";
    for (std::pair<std::string, ResourceMetadata> metadata : m_nonSubresourceTextureMap){
        std::string name = metadata.first;
        int id = metadata.second.resourceId;
        f << "        {\"" << name << "\", " << metadata.second.resourceId << "},\n";
    }
    f << "        // sub-resources\n";
    for (std::pair<std::string, ResourceMetadata> metadata : m_metadataMap){
        std::string name = metadata.first;
        int id = metadata.second.resourceId;
        f << "        {\"" << name << "\", " << metadata.second.resourceId << "},\n";
    }
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
    std::cout << "Writing asset_manifest.json" << std::endl;
    nlohmann::json manifest;

    // get date/time
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%d-%m-%Y %H:%M:%S");
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
        model["name"] = name;
        model["sizeBytes"] = metadata.second.sizeBytes;
        model["type"] = ResourceTypes::typeToString(metadata.second.resourceType);
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
        model["name"] = name;
        model["sizeBytes"] = metadata.second.sizeBytes;
        model["type"] = ResourceTypes::typeToString(metadata.second.resourceType);
        manifest["nonSubresourceTextures"].push_back(model);
    }

    // subresource info
    manifest["subresourceCount"] = m_metadataMap.size();
    manifest["subresources"] = nlohmann::json::array();
    // write subresources
    for (std::pair<std::string, ResourceMetadata> metadata : m_metadataMap){
        std::string name = metadata.first;
        int id = metadata.second.resourceId;
        nlohmann::json subresource;
        subresource["id"] = id;
        subresource["name"] = name;
        subresource["sizeBytes"] = metadata.second.sizeBytes;
        subresource["type"] = ResourceTypes::typeToString(metadata.second.resourceType);
        manifest["subresources"].push_back(subresource);
    }

    // write JSON to file
    std::ofstream f;
    f.open ("../data/asset_manifest.json");
    if (!f.is_open()){
        std::cout << "Failed to create JSON file" << std::endl;
        return;
    }

    // output to file
    f << std::setw(4) << manifest << std::endl;

    // close
    f.close();
}


bool isNumber(const std::string& s){
    for (char const &ch : s) {
        if (std::isdigit(ch) == 0) 
            return false;
    }
    return true;
 }


void AssetRegisterer::registerDirectory(std::string dir){
    int totalSizeBytes = 0;

    // list all models
    std::cout << "Processing the following models: " << std::endl;
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "models/")){
        std::cout << "    " << dirEntry << std::endl;
    }
    std::cout << " " << std::endl;

    // process models and subresources
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "models/")){
        totalSizeBytes += loadModel(dirEntry.path());
    }

    // process non-subresource textures
    for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(dir + "textures/")){
        std::string name = std::filesystem::path(dirEntry).stem();

        size_t found = name.find_last_of("_");
        if (found == std::string::npos){
            totalSizeBytes += loadTexture(dirEntry.path(), false);
            continue;
        }

        std::string tail = name.substr(found+1);
        std::cout << "   ----------------------------   " << tail << std::endl;
        if (isNumber(tail))
            continue;
        
        totalSizeBytes += loadTexture(dirEntry.path(), false);
    }

    // write asset_ids.h
    writeHeader();

    // write asset_manifest.h
    writeManifest(totalSizeBytes);

    std::cout << "Done, processed:" << std::endl;
    std::cout << "   models: " << m_modelMetadataMap.size() << std::endl;
    std::cout << "   subres: " << m_metadataMap.size() << std::endl;
    std::cout << "   bytes:  " << totalSizeBytes << std::endl;
}

}