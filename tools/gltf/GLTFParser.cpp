#include <fstream>
#include "GLTFParser.h"
#include "FileWriter.h"

namespace spr::tools{

GLTFParser::GLTFParser(){}

void GLTFParser::writeBufferFile(const unsigned char* data, uint32_t byteLength, uint32_t elementType, uint32_t componentType, uint32_t bufferId){
    // element type (4)
    // component type (4)
    // byte length (4)
    // data (byte length)
    std::cout << "      Write Buffer File:" << std::endl;

    // write to file
    // open file
    std::ofstream f("../data/buffers/"+(m_name+("_"+std::to_string(bufferId)))+".sbuf", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;

    // write element type
    f.write((char*)&elementType, sizeof(uint32_t));
    std::cout << "          w: elementType: " << elementType << std::endl;

    // write component type
    f.write((char*)&componentType, sizeof(uint32_t));
    std::cout << "          w: componentType: " << componentType << std::endl;

    // write byte length
    f.write((char*)&byteLength, sizeof(uint32_t));
    std::cout << "          w: byteLength: " << byteLength << std::endl;
    
    // write data 
    f.write((char*)data, byteLength);
    std::cout << "          w: data" << std::endl;

    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

void GLTFParser::writeMeshFile(
        uint32_t materialId, 
        int32_t indexBufferId, 
        int32_t posBufferId, 
        int32_t normalBufferId, 
        int32_t colorBufferId, 
        int32_t tangentBufferId, 
        std::vector<uint32_t> texCoordBufferIds,
        uint32_t meshId){
    // material id (4)
    // index buffer id (4)
    // position buffer id (4)
    // normal buffer id (4)
    // color buffer id (4)
    // tangent buffer id (4)
    // texcoord buffer id(s) (4 * n)

    std::cout << "      Write Mesh File:" << std::endl;

    // write to file
    // open file
    std::ofstream f("../data/meshes/"+(m_name+("_"+std::to_string(meshId)))+".smsh", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;

    // write material id
    f.write((char*)&materialId, sizeof(uint32_t));
    std::cout << "          w: materialId: " << materialId << std::endl;

    // write index buffer id
    f.write((char*)&indexBufferId, sizeof(int32_t));
    std::cout << "          w: indexBufferId: " << indexBufferId << std::endl;

    // write position buffer id
    f.write((char*)&posBufferId, sizeof(int32_t));
    std::cout << "          w: posBufferId: " << posBufferId << std::endl;

    // write normal buffer id
    f.write((char*)&normalBufferId, sizeof(int32_t));
    std::cout << "          w: normalBufferId: " << normalBufferId << std::endl;

    // write color buffer id
    f.write((char*)&colorBufferId, sizeof(int32_t));
    std::cout << "          w: colorBufferId: " << colorBufferId << std::endl;

    // write tangent buffer id
    f.write((char*)&tangentBufferId, sizeof(int32_t));
    std::cout << "          w: tangentBufferId: " << tangentBufferId << std::endl;

    // write tex coord buffer ids
    for (uint32_t texCoordBufferId : texCoordBufferIds) {
        f.write((char*)&texCoordBufferId, sizeof(uint32_t));
        std::cout << "          w: texCoordBufferId: " << texCoordBufferId << std::endl;
    }

    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

void GLTFParser::writeMaterialFile(
        uint32_t materialFlags, 
        const tinygltf::Material& material,
        std::vector<uint32_t> texIds,
        uint32_t materialId){
    uint32_t currId = 0;
    std::cout << "      Write Material File:" << std::endl;
    // write to file
    // open file
    std::ofstream f("../data/materials/"+(m_name+("_"+std::to_string(materialId)))+".smtl", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;

    // base color (4)
    //      tex id (4) (2 id, 2 min/mag filter)
    //      factor (16)
    if (materialFlags & 0b1){
        std::cout << "          [base color] " << std::endl;
        // base color sentinel
        uint32_t sentinel = 1;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;
        
        // tex id
        uint32_t baseColorTexId = texIds[currId++];
        f.write((char*)&baseColorTexId, sizeof(uint32_t));
        std::cout << "          w: baseColorTexId: " << (baseColorTexId&0xffff) << std::endl;
        std::cout << "          w: filter: " << ((baseColorTexId>>16)&0xffff) << std::endl;
        
        // base color factor (x,y,z,w)
        std::vector<double> baseColorFactorDouble = material.pbrMetallicRoughness.baseColorFactor;
        std::vector<float> baseColorFactor(baseColorFactorDouble.begin(), baseColorFactorDouble.end());
        //      x
        f.write((char*)&baseColorFactor[0], sizeof(float));
        std::cout << "          w: baseColorFactor.x: " << baseColorFactor[0] << std::endl;
        //      y
        f.write((char*)&baseColorFactor[1], sizeof(float));
        std::cout << "          w: baseColorFactor.y: " << baseColorFactor[1] << std::endl;
        //      z
        f.write((char*)&baseColorFactor[2], sizeof(float));
        std::cout << "          w: baseColorFactor.z: " << baseColorFactor[2] << std::endl;
        //      w
        f.write((char*)&baseColorFactor[3], sizeof(float));
        std::cout << "          w: baseColorFactor.w: " << baseColorFactor[3] << std::endl;
    }

    // metalroughness (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (8) (m(4)/r(4))
    if (materialFlags & (0b1<<1)){
        std::cout << "          [metallic roughness] " << std::endl;
        // metalroughness sentinel
        uint32_t sentinel = 2;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;
        
        // tex id
        uint32_t metalRoughnessTexId = texIds[currId++];
        f.write((char*)&metalRoughnessTexId, sizeof(uint32_t));
        std::cout << "          w: metalRoughnessTexId: " << (metalRoughnessTexId&0xffff) << std::endl;
        std::cout << "          w: filter: " << ((metalRoughnessTexId>>16)&0xffff) << std::endl;
        
        // metalness factor
        float metalFactor = material.pbrMetallicRoughness.metallicFactor;
        f.write((char*)&metalFactor, sizeof(float));
        std::cout << "          w: metalFactor: " << metalFactor << std::endl;
        
        // roughness factor
        float roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        f.write((char*)&roughnessFactor, sizeof(float));
        std::cout << "          w: roughnessFactor: " << roughnessFactor << std::endl;
    }

    // normal (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      scale (4)
    if (materialFlags & (0b1<<2)){
        std::cout << "          [normal] " << std::endl;
        // normal sentinel
        uint32_t sentinel = 3;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;
        
        // tex id
        uint32_t normalTexId = texIds[currId++];
        f.write((char*)&normalTexId, sizeof(uint32_t));
        std::cout << "          w: normalTexId: " << (normalTexId&0xffff) << std::endl;
        std::cout << "          w: filter: " << ((normalTexId>>16)&0xffff) << std::endl;
        
        // normal scale
        float normalScale = material.normalTexture.scale;
        f.write((char*)&normalScale, sizeof(float));
        std::cout << "          w: normalScale: " << normalScale << std::endl;
    }

    // occlusion (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      strength (4)
    if (materialFlags & (0b1<<3)){
        std::cout << "          [occlusion] " << std::endl;
        // occlusion sentinel
        uint32_t sentinel = 4;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;
        
        // tex id
        uint32_t occlusionTexId = texIds[currId++];
        f.write((char*)&occlusionTexId, sizeof(uint32_t));
        std::cout << "          w: occlusionTexId: " << (occlusionTexId&0xffff) << std::endl;
        std::cout << "          w: filter: " << ((occlusionTexId>>16)&0xffff) << std::endl;
        
        // occlusion strength
        float occlusionStrength = material.occlusionTexture.strength;
        f.write((char*)&occlusionStrength, sizeof(float));
        std::cout << "          w: occlusionStrength: " << occlusionStrength << std::endl;
    }

    // emissive (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (12)
    if (materialFlags & (0b1<<4)){
        std::cout << "          [emissive]" << std::endl;
        // emissive sentinel
        uint32_t sentinel = 5;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;

        // tex id
        uint32_t emissiveTexId = texIds[currId++];
        f.write((char*)&emissiveTexId, sizeof(uint32_t));
        std::cout << "          w: emissiveTexId: " << (emissiveTexId&0xffff) << std::endl;
        std::cout << "          w: filter: " << ((emissiveTexId>>16)&0xffff) << std::endl;        

        // emissive factor (x,y,z);
        std::vector<double> emissiveFactorDouble = material.emissiveFactor;
        std::vector<float> emissiveFactor(emissiveFactorDouble.begin(), emissiveFactorDouble.end());
        //      x
        f.write((char*)&emissiveFactor[0], sizeof(float));
        std::cout << "          w: emissiveFactor.x: " << emissiveFactor[0] << std::endl;
        //      y
        f.write((char*)&emissiveFactor[1], sizeof(float));
        std::cout << "          w: emissiveFactor.y: " << emissiveFactor[1] << std::endl;
        //      z
        f.write((char*)&emissiveFactor[2], sizeof(float));
        std::cout << "          w: emissiveFactor.z: " << emissiveFactor[2] << std::endl;
    }

    // alpha (4)
    //      type (4)
    //      cutoff (4)
    if (materialFlags & (0b1<<5)){
        std::cout << "          [alpha] " << std::endl;
        // alpha sentinel
        uint32_t sentinel = 6;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;

        // alpha type
        uint32_t alphaType = 0; // blend by default
        if (material.alphaMode == "MASK")
            alphaType = 1;
        f.write((char*)&alphaType, sizeof(uint32_t));
        std::cout << "          w: alphaType: " << alphaType << std::endl;

        // alpha cutoff
        float alphaCutoff = material.alphaCutoff;
        f.write((char*)&alphaCutoff, sizeof(float));
        std::cout << "          w: alphaCutoff: " << alphaCutoff << std::endl;

    }

    // doublesided (4)
    if (materialFlags & (0b1<<6)){
        std::cout << "          [double sided] " << std::endl;
        // doublesided sentinel
        uint32_t sentinel = 7;
        f.write((char*)&sentinel, sizeof(uint32_t));
        std::cout << "          w: sentinel: " << sentinel << std::endl;
        
    }
    
    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

void GLTFParser::writeModelFile(uint32_t meshCount, std::vector<uint32_t> meshIds, uint32_t modelId){
    // assuming no animations

    // name length (4)
    // name (nameLength)
    // mesh count (4)
    // mesh ids (4*count) 
    uint32_t nameLength = m_name.size();

    std::cout << "      Write Model File:" << std::endl;

    // write to file
    // open file
    std::ofstream f("../data/models/"+m_name+".smdl", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;
    
    // write name length
    f.write((char*)&nameLength, sizeof(uint32_t));
    std::cout << "          w: nameLength: " << nameLength << std::endl;

    // write name
    f.write(m_name.c_str(), nameLength);
    std::cout << "          w: name: " << m_name << std::endl;

    // write mesh count
    f.write((char*)&meshCount, sizeof(uint32_t));
    std::cout << "          w: meshCount: " << meshCount << std::endl;

    // write mesh ids
    for (int i = 0; i < meshCount; i++){
        f.write((char*)&meshIds[i], sizeof(uint32_t));
        std::cout << "          w: meshId: " << meshIds[i] << std::endl;
    }

    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

void GLTFParser::writeTextureFile(uint32_t bufferId, uint32_t imageType, uint32_t texId){
    // buffer id (4)
    // raw/png/jpg/raw/bmp (4)

    std::cout << "      Write Texture File:" << std::endl;

    // write to file
    // open file
    std::ofstream f("../data/textures/"+(m_name+("_"+std::to_string(texId)))+".stex", std::ios::binary);
    if (!f.is_open()){
        // log error
        std::cerr << "Failed to open/create file" << std::endl;
        return ;
    }
    std::cout << "          f: created" << std::endl;
    
    // write buffer id
    f.write((char*)&bufferId, sizeof(uint32_t));
    std::cout << "          w: bufferId: " << bufferId << std::endl;

    // write image type
    f.write((char*)&imageType, sizeof(uint32_t));
    std::cout << "          w: imageType: " << imageType << std::endl;

    // close file
    f.close();
    std::cout << "          f: closed" << std::endl;
    std::cout << "" << std::endl;
}

void GLTFParser::handleBuffer(
        const tinygltf::Buffer& buffer, 
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId){
    std::cout << "Buffer:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  byteLength " << byteLength << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bufferId " << bufferId << std::endl;
    std::cout << "" << std::endl;
    // write slice of buffer int32_to new buffer
    const unsigned char* bufferData = buffer.data.data();
    unsigned char* data = new unsigned char[byteLength];
    for (int32_t i = 0; i < byteLength; i++){
        data[i] = bufferData[i+byteOffset];
    }
    
    // write slice to file
    writeBufferFile(data, byteLength, elementType, componentType, bufferId);

    delete[] data;
}

void GLTFParser::handleBufferint32_terleaved(
        const tinygltf::Buffer& buffer, 
        uint32_t byteOffset, 
        uint32_t byteLength, 
        uint32_t byteStride,
        uint32_t bytesPerElement, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId){
    std::vector<unsigned char> bufferData = buffer.data;
    std::vector<unsigned char> data;
    std::cout << "Bufferint32_terleaved:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  byteLength " << byteLength << std::endl;
    std::cout << "  byteStride " << byteStride << std::endl;
    std::cout << "  bytesPerElement " << bytesPerElement << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bufferId " << bufferId << std::endl;
    std::cout << "" << std::endl;
    // iterate over buffer, one stride at a time 
    for (uint32_t i = byteOffset; i < byteOffset + byteLength; i+= byteStride){
        // grab neccessary bytes from stride (byte-by byte, may be slow)
        for (uint32_t b = 0; b < bytesPerElement; b++){
            data.push_back(bufferData[i+b]);
        }
    }

    // write data to file
    writeBufferFile(data.data(), byteLength, elementType, componentType, bufferId);
}

void GLTFParser::handleBufferView(
        const tinygltf::BufferView& bufferView, 
        uint32_t byteOffset,
        uint32_t bytesPerElement, 
        uint32_t elementCount, 
        uint32_t elementType, 
        uint32_t componentType,
        uint32_t bufferId){
    std::cout << "BufferView:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  bytesPerElement " << bytesPerElement << std::endl;
    std::cout << "  elementCount " << elementCount << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bufferId " << bufferId << std::endl;
    // properties
    uint32_t adjustedByteOffset = bufferView.byteOffset + byteOffset;
    uint32_t byteLength = bufferView.byteLength;
    uint32_t byteStride = bufferView.byteStride;
    if (byteStride == 0)
        byteStride = bytesPerElement;
    std::cout << "  [properties]" << std::endl;
    std::cout << "  bufferView.byteOffset " << bufferView.byteOffset << std::endl;
    std::cout << "  adjustedByteOffset " << adjustedByteOffset << std::endl;
    std::cout << "  byteLength " << byteLength << std::endl;
    std::cout << "  byteStride " << byteStride << std::endl;
    std::cout << "" << std::endl;

    // buffer
    const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

    // handle buffer
    if (byteStride == bytesPerElement)
        handleBuffer(buffer, byteOffset, byteLength, elementType, componentType, bufferId);
    else
        handleBufferint32_terleaved(buffer, byteOffset, byteStride*elementCount, byteStride, bytesPerElement, elementType, componentType, bufferId);
    
}

uint32_t GLTFParser::handleAccessor(const tinygltf::Accessor& accessor){
    uint32_t accessorId = m_id++;

    // properties
    uint32_t byteOffset = accessor.byteOffset;
    uint32_t elementCount = accessor.count;
    uint32_t elementType = accessor.type;
    uint32_t componentType = accessor.componentType;
    uint32_t bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);
    std::cout << "Accessor:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  elementCount " << elementCount << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bytesPerElement " << bytesPerElement << std::endl;
    std::cout << "" << std::endl;

    // buffer view
    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

    // handle bufer view
    handleBufferView(bufferView, byteOffset, bytesPerElement, elementCount, elementType, componentType, accessorId);
    return accessorId;
}

uint32_t GLTFParser::handleTexture(const tinygltf::Texture& tex){
    int32_t texId = m_id++;
    
    // get image and sampler
    int32_t sourceIndex = tex.source;
    int32_t samplerIndex = tex.sampler;
    int32_t minFilter;
    tinygltf::Image image;
    tinygltf::Sampler sampler;
    std::cout << "Texture:" << std::endl;
    std::cout << "  sourceIndex " << sourceIndex << std::endl;
    std::cout << "  samplerIndex " << samplerIndex << std::endl;
    if (sourceIndex == -1)
        return 0;
    if (samplerIndex == -1)
        minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    else {
        sampler = model.samplers[samplerIndex];
        minFilter = sampler.minFilter;
    }

    // create masked id (id + filter)
    uint32_t maskedTexId = (texId & 0xFFFF) | (minFilter << 16);
    
    // tex already written to buffer
    // write tex file but not buffer
    if (m_sourceIdMap.count(sourceIndex) > 0){
        // write texture to file
        std::cout << "  [exists, skipping]" << std::endl;
        std::cout << "" << std::endl;
        writeTextureFile(m_sourceIdMap[sourceIndex], 0, texId);
        return maskedTexId;
    }

    // get image
    image = model.images[sourceIndex];

    // get min filter
    minFilter = sampler.minFilter;

    // get data and write to buffer
    int32_t elementType = TINYGLTF_TYPE_SCALAR;
    int32_t componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    int32_t bufferId = m_id++;
    m_sourceIdMap[sourceIndex] = bufferId;
    if (image.bufferView >= 0){ // bufferview
        int32_t elementCount = image.width * image.height * image.component;
        std::cout << "  [bufferview] " << std::endl;
        std::cout << "  minFilter " << minFilter << std::endl;
        std::cout << "  elementType " << elementType << std::endl;
        std::cout << "  componentType " << componentType << std::endl;
        std::cout << "  elementCount " << elementCount << std::endl;
        handleBufferView(model.bufferViews[image.bufferView], 0, 1, elementCount, elementType, componentType, bufferId);
    } else { // direct buffer
        tinygltf::Buffer buffer;
        int32_t elementCount = image.image.size();
        buffer.data = image.image;
        std::cout << "  [buffer uri] " << std::endl;
        std::cout << "  minFilter " << minFilter << std::endl;
        std::cout << "  elementType " << elementType << std::endl;
        std::cout << "  componentType " << componentType << std::endl;
        std::cout << "  elementCount " << elementCount << std::endl;
        handleBuffer(buffer, 0, elementCount, elementType, componentType, bufferId);
    }
    std::cout << "" << std::endl;

    // write texture to file
    writeTextureFile(bufferId, 0, texId);
    return maskedTexId;
}

uint32_t GLTFParser::handleTexture(const tinygltf::TextureInfo& texInfo){
    // get texture
    int32_t texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex);
}

uint32_t GLTFParser::handleTexture(const tinygltf::NormalTextureInfo& texInfo){
    // get texture
    int32_t texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex);
}

uint32_t GLTFParser::handleTexture(const tinygltf::OcclusionTextureInfo& texInfo){
    // get texture
    int32_t texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex);
}

uint32_t GLTFParser::handleMaterial(const tinygltf::Material& material){
    uint32_t materialId = m_id++;
    std::cout << "Material:" << std::endl;
    // material flags
    uint32_t materialFlags = 0;

    // tex ids
    std::vector<uint32_t> texIds;

    // pbrmetallicroughness
    const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
    if (pbr.baseColorTexture.index >= 0){ // base color
        materialFlags |= 0b1;
        std::cout << "  > basecolor" << std::endl;
        texIds.push_back(handleTexture(pbr.baseColorTexture));
    }
    if (pbr.metallicRoughnessTexture.index >= 0){ //metallicroughness
        materialFlags |= (0b1<<1);
        std::cout << "  > metrough" << std::endl;
        texIds.push_back(handleTexture(pbr.metallicRoughnessTexture));
    }
    // normal
    const tinygltf::NormalTextureInfo& normal = material.normalTexture;
    if (normal.index >= 0){
        materialFlags |= (0b1<<2);
        std::cout << "  > normal" << std::endl;
        texIds.push_back(handleTexture(normal));
    }
    // occlusion
    const tinygltf::OcclusionTextureInfo& occlusion = material.occlusionTexture;
    if (occlusion.index >= 0){
        materialFlags |= (0b1<<3);
        std::cout << "  > occlusion" << std::endl;
        texIds.push_back(handleTexture(occlusion));
    }
    // emissive
    const tinygltf::TextureInfo& emissive = material.emissiveTexture;
    if (emissive.index >= 0){
        materialFlags |= (0b1<<4);
        std::cout << "  > emissive" << std::endl;
        texIds.push_back(handleTexture(emissive));
    }
    // alphamode
    std::string alpha = material.alphaMode;
    if (alpha != "OPAQUE"){
        std::cout << "  > alpha" << std::endl;
        materialFlags |= (0b1<<5);
    }
    // doublesided 
    if (material.doubleSided){
        std::cout << "  > doublesided" << std::endl;
        materialFlags |= (0b1<<6);
    }
    //TODO: check for specularglossiness + update flags
    std::cout << "" << std::endl;
    // write material to file
    writeMaterialFile(materialFlags, material, texIds, materialId);

    return materialId;
}

uint32_t GLTFParser::handlePrimitive(const tinygltf::Primitive& primitive){
    uint32_t meshId = m_id++;

    // material
    int32_t materialIndex = primitive.material;

    // attributes (accessor indices)
    int32_t indicesAccessorIndex = primitive.indices;
    int32_t positionAccessorIndex = -1;
    int32_t normalAccessorIndex = -1;
    int32_t tangentAccessorIndex = -1;
    std::vector<uint32_t> texcoordAccessorIndices;
    std::vector<uint32_t> colorAccessorIndices;

    // iterate over primitive's attributes
    for (auto const& [key, val] : primitive.attributes){
        if (key == "POSITION"){
            positionAccessorIndex = val;
        } else if (key == "NORMAL"){
            normalAccessorIndex = val;
        } else if (key == "TANGENT"){
            tangentAccessorIndex = val;
        } else if (key.find("TEXCOORD") != std::string::npos){
            texcoordAccessorIndices.push_back(val);
        } else if (key.find("COLOR") != std::string::npos){
            colorAccessorIndices.push_back(val);
        }
    }
    std::cout << "Primitive:" << std::endl;

    // handle accessors
    // indices
    int32_t indicesId = handleAccessor(model.accessors[indicesAccessorIndex]);

    // position
    int32_t positionId = -1;
    if (positionAccessorIndex >= 0){
        positionId = handleAccessor(model.accessors[positionAccessorIndex]);
    }

    // normal
    int32_t normalId = -1;
    if (normalAccessorIndex >= 0){
        normalId = handleAccessor(model.accessors[normalAccessorIndex]);
    }

    // tangent
    int32_t tangentId = -1;
    if (tangentAccessorIndex >= 0){
        tangentId = handleAccessor(model.accessors[tangentAccessorIndex]);
    }

    // texcoords
    std::vector<uint32_t> texCoordIds;
    for (auto & texcoordAccessorIndex : texcoordAccessorIndices) {
        texCoordIds.push_back(handleAccessor(model.accessors[texcoordAccessorIndex]));
    }

    // colors
    int32_t colorId = -1;
    for (auto & colorAccessorIndex : colorAccessorIndices) {
        colorId = handleAccessor(model.accessors[colorAccessorIndex]);
        break;
    }

    //std::cout << "Primitive:" << std::endl;
    std::cout << "  materialIndex " << materialIndex << std::endl;
    std::cout << "  indicesAccessorIndex " << indicesAccessorIndex << std::endl;
    std::cout << "  positionAccessorIndex " << positionAccessorIndex << std::endl;
    std::cout << "  normalAccessorIndex " << normalAccessorIndex << std::endl;
    std::cout << "  tangentAccessorIndex " << tangentAccessorIndex << std::endl;
    std::cout << "  indicesId " << indicesId << std::endl;
    std::cout << "  positionId " << positionId << std::endl;
    std::cout << "  normalId " << normalId << std::endl;
    std::cout << "  tangentId " << tangentId << std::endl;
    std::cout << "  colorId " << colorId << std::endl;
    std::cout << "" << std::endl;

    // handle material
    uint32_t materialId = 0;
    if (materialIndex >= 0)
        materialId = handleMaterial(model.materials[materialIndex]);

    // write prim (mesh) to file
    writeMeshFile(materialId, indicesId, positionId, normalId, colorId, tangentId, texCoordIds, meshId);

    return meshId;
}

void GLTFParser::handleMesh(const tinygltf::Mesh& mesh, std::vector<uint32_t> &meshIds){
    for (int32_t i = 0; i < mesh.primitives.size(); i++){
        const tinygltf::Primitive& primitive = mesh.primitives[i];
        if (primitive.mode == 4 || primitive.mode == -1){
            meshIds.push_back(handlePrimitive(primitive));
        }
    }
}

void GLTFParser::parseNode(const tinygltf::Node& node, std::vector<uint32_t> &meshIds){
    // handle meshes
    if (node.mesh != -1){
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        handleMesh(mesh, meshIds);
    }

    // handle children
    for (int32_t i = 0; i < node.children.size(); i++){
        const tinygltf::Node& child = model.nodes[node.children[i]];
        parseNode(child, meshIds);
    }
}

void GLTFParser::parse(){
    std::cout << "Sucessfully loaded, parsing..." << std::endl;

    int32_t modelId = m_id++;
    // assume one scene
    const tinygltf::Scene& scene = model.scenes[0];
    std::vector<uint32_t> meshIds;
    // process top level nodes
    for (int32_t i = 0; i < scene.nodes.size(); i++){
        const tinygltf::Node& currNode = model.nodes[scene.nodes[i]];
        parseNode(currNode, meshIds);
    }
    // write model file
    writeModelFile(meshIds.size(), meshIds, modelId);
}

// parse .gltf file
void GLTFParser::parseJson(std::string path){
    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse .gltf\n");
        return;
    }

    m_path = std::filesystem::path(path).parent_path().concat("/");
    m_name = std::filesystem::path(path).stem();
    m_extension = std::filesystem::path(path).extension();

    parse();
}

// parse .glb file
void GLTFParser::parseBinary(std::string path){
    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse .glb\n");
        return;
    }   

    m_path = std::filesystem::path(path).parent_path().concat("/");
    m_name = std::filesystem::path(path).stem();
    m_extension = std::filesystem::path(path).extension();

    parse();
}

}