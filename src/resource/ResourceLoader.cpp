#include "ResourceLoader.h"

namespace spr{

ResourceLoader::ResourceLoader(){

}

template <typename T>
T ResourceLoader::loadFromMetadata(ResourceMetadata metadata){
    // log unknown resource
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
template <>
Model ResourceLoader::loadFromMetadata<Model>(ResourceMetadata metadata){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        // log error
        return Model();
    }

    uint32 nameLength;
    char* name;
    uint32 meshCount;

    // read name length
    f.read((char*)&nameLength, sizeof(uint32));

    // read name
    name = (char*)malloc(nameLength*sizeof(char));
    f.read(name, nameLength);

    // read mesh count
    f.read((char*)&meshCount, sizeof(uint32));

    // read mesh ids
    std::vector<uint32> meshIds(meshCount);
    for (int i = 0; i < meshCount; i++){
        f.read((char*)&meshIds[i], sizeof(uint32));
    }

    // close file
    f.close();

    // ---- create Model ----
    Model model;
    // instance data
    model.id = m_instanceId++;
    model.resourceId = metadata.resourceId;
    // resource data
    model.meshCount = meshCount;
    model.meshIds = meshIds;

    return model;
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
template <>
Mesh ResourceLoader::loadFromMetadata<Mesh>(ResourceMetadata metadata){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        // log error
        return Mesh();
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

    // read position buffer id
    f.read((char*)&positionBufferId, sizeof(int32));

    // read normal buffer id
    f.read((char*)&normalBufferId, sizeof(int32));

    // read color buffer id
    f.read((char*)&colorBufferId, sizeof(int32));

    // read tangent buffer id
    f.read((char*)&tangentBufferId, sizeof(int32));

    // read tex coord buffer ids
    int32 index = 0;
    while (!f.eof()){
        texCoordBufferIds.push_back(-1);
        f.read((char*)&texCoordBufferIds[index], sizeof(int32));
        index++;
    }

    // close file
    f.close();

    // ---- create Mesh ----
    Mesh mesh;
    // instance data
    mesh.id = m_instanceId++;
    mesh.resourceId = metadata.resourceId;
    // resource data
    mesh.materialId = materialId;
    mesh.indexBufferId = indexBufferId;
    mesh.positionBufferId = positionBufferId;
    mesh.normalBufferId = normalBufferId;
    mesh.colorBufferId = colorBufferId;
    mesh.tangentBufferId = tangentBufferId;
    mesh.texCoordBufferIds = texCoordBufferIds;

    return mesh;
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
template <>
Material ResourceLoader::loadFromMetadata<Material>(ResourceMetadata metadata){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        // log error
        return Material();
    }

    uint32 materialFlags;

    int32 baseColorTexId;
    glm::vec4 baseColorFactor;

    int32 metalRoughTexId;
    float metalFactor;
    float roughnessFactor;

    int32 normalTexId;
    float normalScale;

    int32 occlusionTexId;
    float occlusionStrength;

    int32 emissiveTexId;
    glm::vec3 emissiveFactor;

    int32 alphaType;
    float alphaCutoff;

    bool doubleSided;

    while(!f.eof()){
        uint32 materialType;
        f.read((char*)&materialType, sizeof(uint32));

        switch(materialType) {
            case 1 : // base color
                materialFlags |= 0b1;
                f.read((char*)&baseColorTexId, sizeof(int32));
                f.read((char*)&baseColorFactor.x, sizeof(float));
                f.read((char*)&baseColorFactor.y, sizeof(float));
                f.read((char*)&baseColorFactor.z, sizeof(float));
                f.read((char*)&baseColorFactor.w, sizeof(float));
                break;
            case 2 : // metalroughness
                materialFlags |= (0b1<<1);
                f.read((char*)&metalRoughTexId, sizeof(int32));
                f.read((char*)&metalFactor, sizeof(float));
                f.read((char*)&roughnessFactor, sizeof(float));
                break;
            case 3 : // normal
                materialFlags |= (0b1<<2);
                f.read((char*)&normalTexId, sizeof(int32));
                f.read((char*)&normalScale, sizeof(float));
                break;
            case 4 : // occlusion
                materialFlags |= (0b1<<3);
                f.read((char*)&occlusionTexId, sizeof(int32));
                f.read((char*)&occlusionStrength, sizeof(float));
                break;
            case 5 : // emissive
                materialFlags |= (0b1<<4);
                f.read((char*)&emissiveTexId, sizeof(int32));
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

    // ---- create Material ----
    Material material;
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
    return material;
}


// ------------------------------------------------------------------------- //
//    Texture - .stex                                                        // 
// ------------------------------------------------------------------------- //
// ╔═══════════════════════════════════╗
// ║     buffer id (4)                 ║ // buffer holding tex data
// ╠═══════════════════════════════════╣
// ║     image type (4)                ║ // 0-raw/1-png/2-jpg (0 only)
// ╚═══════════════════════════════════╝
template <>
Texture ResourceLoader::loadFromMetadata<Texture>(ResourceMetadata metadata){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        // log error
        return Texture();
    }

    uint32 bufferId;
    uint32 imageType;

    // read buffer id
    f.read((char*)&bufferId, sizeof(uint32));

    // read image type
    f.read((char*)&imageType, sizeof(uint32));

    // close file
    f.close();

    // ---- create Texture ----
    Texture texture;
    // instance data
    texture.id = m_instanceId++;
    texture.resourceId = metadata.resourceId;
    // resource data
    texture.bufferId = bufferId;
    texture.imageType = imageType;

    return texture;
}


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
template <>
Buffer ResourceLoader::loadFromMetadata<Buffer>(ResourceMetadata metadata){
    // open file
    std::ifstream f(
        ResourceTypes::getPath(metadata.resourceType)+
        metadata.name+
        ResourceTypes::getExtension(metadata.resourceType), 
        std::ios::binary);
    if (!f.is_open()){
        // log error
        return Buffer();
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

    // close file
    f.close();

    // ---- create Buffer ----
    Buffer buffer;
    // instance data
    buffer.id = m_instanceId++;
    buffer.resourceId = metadata.resourceId;
    // resource data
    buffer.elementType = elementType;
    buffer.componentType = componentType;
    buffer.byteLength = byteLength;
    buffer.data = data;

    return buffer;
}


// ----------------------------------------------------------------------------
//    Audio - .***
//    iw
template <>
Audio ResourceLoader::loadFromMetadata<Audio>(ResourceMetadata metadata){
    return Audio();
}


// ----------------------------------------------------------------------------
//    Shader - .hlsl
//    iw
template <>
Shader ResourceLoader::loadFromMetadata<Shader>(ResourceMetadata metadata){
    return Shader();
}

}