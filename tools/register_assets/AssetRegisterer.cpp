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
// ║     image type (4)                ║ // 0-raw/1-png/2-jpg (0 only)
// ╚═══════════════════════════════════╝
int AssetRegisterer::loadTexture(std::string path){
    std::cout << "    Registering texture:  " << path << std::endl;
    // open file
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to load." << std::endl;
        return 0;
    }

    uint32 bufferId;
    uint32 imageType;

    // read buffer id
    f.read((char*)&bufferId, sizeof(uint32));

    // read image type
    f.read((char*)&imageType, sizeof(uint32));

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
    
    totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(bufferId) + ResourceTypes::getExtension(SPR_BUFFER)); 

    metadata.sizeBytes = totalBytes;
    // store metadata
    m_metadataMap[name] = metadata;

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

    int32 baseColorTexId = -1;
    glm::vec4 baseColorFactor;

    int32 metalRoughTexId = -1;
    float metalFactor;
    float roughnessFactor;

    int32 normalTexId = -1;
    float normalScale;

    int32 occlusionTexId = -1;
    float occlusionStrength;

    int32 emissiveTexId = -1;
    glm::vec3 emissiveFactor;

    int32 alphaType;
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
                f.read((char*)&baseColorTexId, sizeof(int32));
                baseColorTexId &= 0xffff;
                f.read((char*)&baseColorFactor.x, sizeof(float));
                f.read((char*)&baseColorFactor.y, sizeof(float));
                f.read((char*)&baseColorFactor.z, sizeof(float));
                f.read((char*)&baseColorFactor.w, sizeof(float));
                break;
            case 2 : // metalroughness
                materialFlags |= (0b1<<1);
                f.read((char*)&metalRoughTexId, sizeof(int32));
                metalRoughTexId &= 0xffff;
                f.read((char*)&metalFactor, sizeof(float));
                f.read((char*)&roughnessFactor, sizeof(float));
                break;
            case 3 : // normal
                materialFlags |= (0b1<<2);
                f.read((char*)&normalTexId, sizeof(int32));
                normalTexId &= 0xffff;
                f.read((char*)&normalScale, sizeof(float));
                break;
            case 4 : // occlusion
                materialFlags |= (0b1<<3);
                f.read((char*)&occlusionTexId, sizeof(int32));
                occlusionTexId &= 0xffff;
                f.read((char*)&occlusionStrength, sizeof(float));
                break;
            case 5 : // emissive
                materialFlags |= (0b1<<4);
                f.read((char*)&emissiveTexId, sizeof(int32));
                emissiveTexId &= 0xffff;
                f.read((char*)&emissiveFactor.x, sizeof(float));
                f.read((char*)&emissiveFactor.y, sizeof(float));
                f.read((char*)&emissiveFactor.z, sizeof(float));
                break;
            case 6 : // alpha
                materialFlags |= (0b1<<5);
                f.read((char*)&alphaType, sizeof(int32));
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
    
    if (materialFlags & 0b1 && baseColorTexId >= 0){ // base color
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(baseColorTexId) + ResourceTypes::getExtension(SPR_TEXTURE));
    }
    if (materialFlags & (0b1<<1) && metalRoughTexId >= 0){ // metallicroughness
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(metalRoughTexId) + ResourceTypes::getExtension(SPR_TEXTURE));
    }
    if (materialFlags & (0b1<<2) && normalTexId >= 0){ // normal
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(normalTexId) + ResourceTypes::getExtension(SPR_TEXTURE));
    }
    if (materialFlags & (0b1<<3) && occlusionTexId >= 0){ // occlusion
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(occlusionTexId) + ResourceTypes::getExtension(SPR_TEXTURE));
    }
    if (materialFlags & (0b1<<4) && emissiveTexId >= 0){ // emissive
        totalBytes += loadTexture(ResourceTypes::getPath(SPR_TEXTURE) + name.substr(0,found) + "_" + std::to_string(emissiveTexId) + ResourceTypes::getExtension(SPR_TEXTURE));
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
// ║     normal buffer id (4)          ║ // normal buffer file
// ╠═══════════════════════════════════╣
// ║     color buffer id (4)           ║ // color buffer file
// ╠═══════════════════════════════════╣
// ║     tangent buffer id (4)         ║ // tangent buffer file
// ╠═══════════════════════════════════╣ 
// ║                 ...               ║
// ╠     texcoord buffer id(s) (4 * n) ╣ // texcoord buffer file(s)
// ║                 ...               ║ // should equal # of textures in material
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
    int32 indexBufferId;
    int32 positionBufferId;
    int32 normalBufferId;
    int32 colorBufferId;
    int32 tangentBufferId;
    std::vector<int32> texCoordBufferIds;

    // read material id
    f.read((char*)&materialId, sizeof(uint32));

    // read index buffer id
    f.read((char*)&indexBufferId, sizeof(int32));
    if (indexBufferId >= 0)
        indexBufferId &= 0xffff;

    // read position buffer id
    f.read((char*)&positionBufferId, sizeof(int32));
    if (positionBufferId >= 0)
        positionBufferId &= 0xffff;

    // read normal buffer id
    f.read((char*)&normalBufferId, sizeof(int32));
    if (normalBufferId >= 0)
        normalBufferId &= 0xffff;

    // read color buffer id
    f.read((char*)&colorBufferId, sizeof(int32));
    if (colorBufferId >= 0)
        colorBufferId &= 0xffff;

    // read tangent buffer id
    f.read((char*)&tangentBufferId, sizeof(int32));
    if (tangentBufferId >= 0)
        tangentBufferId &= 0xffff;

    // read tex coord buffer ids
    int32 index = 0;
    while (!f.eof()){
        texCoordBufferIds.push_back(-1);
        f.read((char*)&texCoordBufferIds[index], sizeof(int32));
        if (f.eof())
            texCoordBufferIds.pop_back();
        index++;
    }

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
    totalBytes += loadMaterial(ResourceTypes::getPath(SPR_MATERIAL) + name.substr(0,found) + "_" + std::to_string(materialId) + ResourceTypes::getExtension(SPR_MATERIAL));
    // index
    if (indexBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(indexBufferId) + ResourceTypes::getExtension(SPR_BUFFER));
    // position
    if (positionBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(positionBufferId) + ResourceTypes::getExtension(SPR_BUFFER));
    // normal
    if (normalBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(normalBufferId) + ResourceTypes::getExtension(SPR_BUFFER));  
    // color
    if (colorBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(colorBufferId) + ResourceTypes::getExtension(SPR_BUFFER));
    // tangent
    if (tangentBufferId > 0)
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(tangentBufferId) + ResourceTypes::getExtension(SPR_BUFFER));
    // texCoord
    for (int id : texCoordBufferIds){
        totalBytes += loadBuffer(ResourceTypes::getPath(SPR_BUFFER) + name.substr(0,found) + "_" + std::to_string(id) + ResourceTypes::getExtension(SPR_BUFFER)); 
    }
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
    for (int i = 0; i < meshCount; i++){
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
    for (int id : meshIds){
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