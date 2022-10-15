#include <fstream>
#include "GLTFParser.h"
#include "FileWriter.h"

namespace spr::tools{

GLTFParser::GLTFParser(){}

void GLTFParser::writeBufferFile(const unsigned char* data, int byteLength, int elementType, int componentType, int bufferId){
    // name.sbuf
    // calculate name as ????
    // write to data/buffer/modelname/????.sbuf

    // element type (4)
    // component type (4)
    // byte length (4)
    // data (byte length)
    std::cout << "      Write Buffer File:" << std::endl;
    std::cout << "          byteLength " << byteLength << std::endl;
    std::cout << "          elementType " << elementType << std::endl;
    std::cout << "          componentType " << componentType << std::endl;
    std::cout << "          bufferId " << bufferId << std::endl;
    std::cout << "" << std::endl;

    unsigned char writeData[byteLength + 4 + 4 + 8];
    // write element type
    writeData[0] = elementType & 0xff;
    writeData[1] = (elementType >> 8) & 0xff;
    writeData[2] = (elementType >> 16) & 0xff;
    writeData[3] = (elementType >> 24) & 0xff;
    // write component type
    writeData[4] = componentType & 0xff;
    writeData[5] = (componentType >> 8) & 0xff;
    writeData[6] = (componentType >> 16) & 0xff;
    writeData[7] = (componentType >> 24) & 0xff;
    // write byte length
    writeData[8] = byteLength & 0xff;
    writeData[9] = (byteLength >> 8) & 0xff;
    writeData[10] = (byteLength >> 16) & 0xff;
    writeData[11] = (byteLength >> 24) & 0xff;
    const unsigned char test = data[0];
    // write data 
    //std::copy(data, data + byteLength, writeData + 12);
    for (int i = 0; i < byteLength; i++){
        writeData[i+12] = data[i];
    }
    // write buffer to file
    FileWriter fw = FileWriter();
    fw.writeFile("../data/buffers/", m_name+("_"+std::to_string(bufferId)), ".sbuf", writeData, byteLength+12);
}

void GLTFParser::writeMeshFile(
        int materialId, 
        int indexBufferId, 
        int posBufferId, 
        int normalBufferId, 
        int colorBufferId, 
        int tangentBufferId, 
        std::vector<int> texCoordBufferIds,
        int meshId){
    // material id (4)
    // index buffer id (4)
    // position buffer id (4)
    // normal buffer id (4)
    // color buffer id (4)
    // tangent buffer id (4)
    // texcoord buffer id(s) (4 * n)
    int byteLength = 4+4+4+4+4+4+4*texCoordBufferIds.size();
    unsigned char writeData[byteLength];

    std::cout << "      Write Mesh File:" << std::endl;
    std::cout << "          byteLength " << byteLength << std::endl;
    std::cout << "          materialId " << materialId << std::endl;
    std::cout << "          indexBufferId " << indexBufferId << std::endl;
    std::cout << "          posBufferId " << posBufferId << std::endl;
    std::cout << "          normalBufferId " << normalBufferId << std::endl;
    std::cout << "          colorBufferId " << colorBufferId << std::endl;
    std::cout << "          tangentBufferId " << tangentBufferId << std::endl;
    std::cout << "          meshId " << meshId << std::endl;
    std::cout << "" << std::endl;

    // write material id
    writeData[0] = materialId & 0xff;
    writeData[1] = (materialId >> 8) & 0xff;
    writeData[2] = (materialId >> 16) & 0xff;
    writeData[3] = (materialId >> 24) & 0xff;

    // write index buffer id
    writeData[4] = indexBufferId & 0xff;
    writeData[5] = (indexBufferId >> 8) & 0xff;
    writeData[6] = (indexBufferId >> 16) & 0xff;
    writeData[7] = (indexBufferId >> 24) & 0xff;

    // write position buffer id
    writeData[8] = posBufferId & 0xff;
    writeData[9] = (posBufferId >> 8) & 0xff;
    writeData[10] = (posBufferId >> 16) & 0xff;
    writeData[11] = (posBufferId >> 24) & 0xff;

    // write normal buffer id
    writeData[12] = normalBufferId & 0xff;
    writeData[13] = (normalBufferId >> 8) & 0xff;
    writeData[14] = (normalBufferId >> 16) & 0xff;
    writeData[15] = (normalBufferId >> 24) & 0xff;

    // write color buffer id
    writeData[16] = colorBufferId & 0xff;
    writeData[17] = (colorBufferId >> 8) & 0xff;
    writeData[18] = (colorBufferId >> 16) & 0xff;
    writeData[19] = (colorBufferId >> 24) & 0xff;

    // write tangent buffer id
    writeData[20] = tangentBufferId & 0xff;
    writeData[21] = (tangentBufferId >> 8) & 0xff;
    writeData[22] = (tangentBufferId >> 16) & 0xff;
    writeData[23] = (tangentBufferId >> 24) & 0xff;

    // write tex coord buffer ids
    int count = 0;
    for (auto & texCoordBufferId : texCoordBufferIds) {
        writeData[24 + 4*count] = tangentBufferId & 0xff;
        writeData[25 + 4*count + 1] = (tangentBufferId >> 8) & 0xff;
        writeData[26 + 4*count + 2] = (tangentBufferId >> 16) & 0xff;
        writeData[27 + 4*count + 3] = (tangentBufferId >> 24) & 0xff;
    }

    // write to file
    FileWriter fw = FileWriter();
    fw.writeFile("../data/meshes/", m_name+("_"+std::to_string(meshId)), ".smsh", writeData, byteLength);
}

void GLTFParser::writeMaterialFile(
        int materialFlags, 
        const tinygltf::Material& material,
        std::vector<int> texIds,
        int materialId){
    int currId = 0;
    std::vector<unsigned char> writeData;
    // base color (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (16)
    std::cout << "      Write Material File:" << std::endl;
    
    if (materialFlags & 0b1){
        std::cout << "          base color " << std::endl;
        // base color sentinel
        writeData.push_back( 1 & 0b1);
        writeData.push_back((1 >> 8) & 0b1);
        writeData.push_back((1 >> 16) & 0b1);
        writeData.push_back((1 >> 24) & 0b1);
        // tex id
        int baseColorTexId = texIds[currId++];
        writeData.push_back(baseColorTexId & 0b1);
        writeData.push_back((baseColorTexId >> 8) & 0b1);
        writeData.push_back((baseColorTexId >> 16) & 0b1);
        writeData.push_back((baseColorTexId >> 24) & 0b1); //filter
        // base color factor (x,y,z,w)
        std::vector<double> baseColorFactorDouble = material.pbrMetallicRoughness.baseColorFactor;
        std::vector<float> baseColorFactor(baseColorFactorDouble.begin(), baseColorFactorDouble.end());
        //      x
        unsigned char const * x = reinterpret_cast<unsigned char const *>(&baseColorFactor[0]);
        writeData.push_back(x[0]);
        writeData.push_back(x[1]);
        writeData.push_back(x[2]);
        writeData.push_back(x[3]);
        //      y
        unsigned char const * y = reinterpret_cast<unsigned char const *>(&baseColorFactor[1]);
        writeData.push_back(y[0]);
        writeData.push_back(y[1]);
        writeData.push_back(y[2]);
        writeData.push_back(y[3]);
        //      z
        unsigned char const * z = reinterpret_cast<unsigned char const *>(&baseColorFactor[2]);
        writeData.push_back(z[0]);
        writeData.push_back(z[1]);
        writeData.push_back(z[2]);
        writeData.push_back(z[3]);
        //      w
        unsigned char const * w = reinterpret_cast<unsigned char const *>(&baseColorFactor[3]);
        writeData.push_back(w[0]);
        writeData.push_back(w[1]);
        writeData.push_back(w[2]);
        writeData.push_back(w[3]);
    }

    // metalroughness (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (8) (m(4)/r(4))
    if (materialFlags & (0b1<<1)){
        std::cout << "          metrough " << std::endl;
        // metalroughness sentinel
        writeData.push_back( 2 & 0b1);
        writeData.push_back((2 >> 8) & 0b1);
        writeData.push_back((2 >> 16) & 0b1);
        writeData.push_back((2 >> 24) & 0b1);
        // tex id
        int metalRoughnessTexId = texIds[currId++];
        writeData.push_back(metalRoughnessTexId & 0b1);
        writeData.push_back((metalRoughnessTexId >> 8) & 0b1);
        writeData.push_back((metalRoughnessTexId >> 16) & 0b1);
        writeData.push_back((metalRoughnessTexId >> 24) & 0b1); //filter
        // metalness factor
        float metalFactor = material.pbrMetallicRoughness.metallicFactor;
        unsigned char const * mF = reinterpret_cast<unsigned char const *>(&metalFactor);
        writeData.push_back(mF[0]);
        writeData.push_back(mF[1]);
        writeData.push_back(mF[2]);
        writeData.push_back(mF[3]);
        // roughness factor
        float roughnessFactor = material.pbrMetallicRoughness.roughnessFactor;
        unsigned char const * rF = reinterpret_cast<unsigned char const *>(&roughnessFactor);
        writeData.push_back(rF[0]);
        writeData.push_back(rF[1]);
        writeData.push_back(rF[2]);
        writeData.push_back(rF[3]);
    }

    // normal (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      scale (4)
    if (materialFlags & (0b1<<2)){
        std::cout << "          normal " << std::endl;
        // normal sentinel
        writeData.push_back( 3 & 0b1);
        writeData.push_back((3 >> 8) & 0b1);
        writeData.push_back((3 >> 16) & 0b1);
        writeData.push_back((3 >> 24) & 0b1);
        // tex id
        int normalTexId = texIds[currId++];
        writeData.push_back(normalTexId & 0b1);
        writeData.push_back((normalTexId >> 8) & 0b1);
        writeData.push_back((normalTexId >> 16) & 0b1);
        writeData.push_back((normalTexId >> 24) & 0b1); //filter
        // normal scale
        float normalScale = material.normalTexture.scale;
        unsigned char const * nS = reinterpret_cast<unsigned char const *>(&normalScale);
        writeData.push_back(nS[0]);
        writeData.push_back(nS[1]);
        writeData.push_back(nS[2]);
        writeData.push_back(nS[3]);
    }

    // occlusion (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      strength (4)
    if (materialFlags & (0b1<<3)){
        std::cout << "          occlusion " << std::endl;
        // occlusion sentinel
        writeData.push_back( 4 & 0b1);
        writeData.push_back((4 >> 8) & 0b1);
        writeData.push_back((4 >> 16) & 0b1);
        writeData.push_back((4 >> 24) & 0b1);
        // tex id
        int occlusionTexId = texIds[currId++];
        writeData.push_back(occlusionTexId & 0b1);
        writeData.push_back((occlusionTexId >> 8) & 0b1);
        writeData.push_back((occlusionTexId >> 16) & 0b1);
        writeData.push_back((occlusionTexId >> 24) & 0b1); //filter
        // occlusion strength
        float occlusionStrength = material.occlusionTexture.strength;
        unsigned char const * oS = reinterpret_cast<unsigned char const *>(&occlusionStrength);
        writeData.push_back(oS[0]);
        writeData.push_back(oS[1]);
        writeData.push_back(oS[2]);
        writeData.push_back(oS[3]);
    }

    // emissive (4)
    //      tex id (4) (3 id, 1 min/mag filter)
    //      factor (12)
    if (materialFlags & (0b1<<4)){
        std::cout << "          emissive" << std::endl;
        // emissive sentinel
        writeData.push_back( 5 & 0b1);
        writeData.push_back((5 >> 8) & 0b1);
        writeData.push_back((5 >> 16) & 0b1);
        writeData.push_back((5 >> 24) & 0b1);
        // tex id
        int emissiveTexId = texIds[currId++];
        writeData.push_back(emissiveTexId & 0b1);
        writeData.push_back((emissiveTexId >> 8) & 0b1);
        writeData.push_back((emissiveTexId >> 16) & 0b1);
        writeData.push_back((emissiveTexId >> 24) & 0b1); //filter
        // emissive factor (x,y,z);
        std::vector<double> emissiveFactorDouble = material.emissiveFactor;
        std::vector<float> emissiveFactor(emissiveFactorDouble.begin(), emissiveFactorDouble.end());
        //      x
        unsigned char const * x = reinterpret_cast<unsigned char const *>(&emissiveFactor[0]);
        writeData.push_back(x[0]);
        writeData.push_back(x[1]);
        writeData.push_back(x[2]);
        writeData.push_back(x[3]);
        //      y
        unsigned char const * y = reinterpret_cast<unsigned char const *>(&emissiveFactor[1]);
        writeData.push_back(y[0]);
        writeData.push_back(y[1]);
        writeData.push_back(y[2]);
        writeData.push_back(y[3]);
        //      z
        unsigned char const * z = reinterpret_cast<unsigned char const *>(&emissiveFactor[2]);
        writeData.push_back(z[0]);
        writeData.push_back(z[1]);
        writeData.push_back(z[2]);
        writeData.push_back(z[3]);
    }

    // alpha (4)
    //      type (4)
    //      cutoff (4)
    if (materialFlags & (0b1<<5)){
        std::cout << "          alpha " << std::endl;
        // alpha sentinel
        writeData.push_back( 6 & 0b1);
        writeData.push_back((6 >> 8) & 0b1);
        writeData.push_back((6 >> 16) & 0b1);
        writeData.push_back((6 >> 24) & 0b1);
        // alpha type
        int alphaType = 0; // blend by default
        if (material.alphaMode == "MASK")
            alphaType = 1;
        writeData.push_back(alphaType & 0b1);
        writeData.push_back((alphaType >> 8) & 0b1);
        writeData.push_back((alphaType >> 16) & 0b1);
        writeData.push_back((alphaType >> 24) & 0b1);
        // alpha cutoff
        float alphaCutoff = material.alphaCutoff;
        unsigned char const * aC = reinterpret_cast<unsigned char const *>(&alphaCutoff);
        writeData.push_back(aC[0]);
        writeData.push_back(aC[1]);
        writeData.push_back(aC[2]);
        writeData.push_back(aC[3]);

    }

    // doublesided (4)
    if (materialFlags & (0b1<<6)){
        std::cout << "          double sided " << std::endl;
        // doublesided sentinel
        writeData.push_back( 7 & 0b1);
        writeData.push_back((7 >> 8) & 0b1);
        writeData.push_back((7 >> 16) & 0b1);
        writeData.push_back((7 >> 24) & 0b1);
    }
    std::cout << "" << std::endl;
    // write to file
    FileWriter fw = FileWriter();
    fw.writeFile("../data/materials/", m_name+("_"+std::to_string(materialId)), ".smtl", writeData.data(), writeData.size());
}

void GLTFParser::writeModelFile(int meshCount, std::vector<int> meshIds, int modelId){
    // assuming no animations

    // name length (4)
    // name (nameLength)
    // mesh count (4)
    // mesh ids (4*count) 
    int nameLength = m_name.size();
    int byteLength = nameLength + 4 + 4 + 4*meshIds.size();
    unsigned char writeData[byteLength];

    // write name length
    writeData[0] = nameLength & 0xff;
    writeData[1] = (nameLength >> 8) & 0xff;
    writeData[2] = (nameLength >> 16) & 0xff;
    writeData[3] = (nameLength >> 24) & 0xff;

    // write name
    int index = 4;
    for (auto & ch : m_name) {
        writeData[index++] = ch;
    }

    // write mesh count
    writeData[nameLength+4] = meshCount & 0xff;
    writeData[nameLength+5] = (meshCount >> 8) & 0xff;
    writeData[nameLength+6] = (meshCount >> 16) & 0xff;
    writeData[nameLength+7] = (meshCount >> 24) & 0xff;

    // write mesh ids
    int count = 0;
    for (auto & meshId : meshIds) {
        writeData[nameLength+8 + 4*count] = meshId & 0xff;
        writeData[nameLength+9 + 4*count + 1] = (meshId >> 8) & 0xff;
        writeData[nameLength+10 + 4*count + 2] = (meshId >> 16) & 0xff;
        writeData[nameLength+11 + 4*count + 3] = (meshId >> 24) & 0xff;
    }

    // write to file
    FileWriter fw = FileWriter();
    fw.writeFile("../data/models/", m_name+("_"+std::to_string(modelId)), ".smdl", writeData, byteLength);
}

void GLTFParser::writeTextureFile(int bufferId, int imageType, int texId){
    // buffer id (4)
    // raw/png/jpg/raw/bmp (4)
    int byteLength = 8;
    unsigned char writeData[byteLength];

    // write buffer id
    writeData[0] = bufferId & 0xff;
    writeData[1] = (bufferId >> 8) & 0xff;
    writeData[2] = (bufferId >> 16) & 0xff;
    writeData[3] = (bufferId >> 24) & 0xff;

    // write image type
    writeData[4] = imageType & 0xff;
    writeData[5] = (imageType >> 8) & 0xff;
    writeData[6] = (imageType >> 16) & 0xff;
    writeData[7] = (imageType >> 24) & 0xff;

    // write to file
    FileWriter fw = FileWriter();
    fw.writeFile("../data/textures/", m_name+("_"+std::to_string(texId)), ".stex", writeData, byteLength+1);
}

void GLTFParser::handleBuffer(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength, 
        int elementType, 
        int componentType,
        int bufferId){
    std::cout << "Buffer:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  byteLength " << byteLength << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bufferId " << bufferId << std::endl;
    std::cout << "" << std::endl;
    // write slice of buffer into new buffer
    const unsigned char* bufferData = buffer.data.data();
    unsigned char* data = new unsigned char[byteLength];
    //std::copy(&bufferData+byteOffset, &bufferData+byteOffset+byteLength, &data);
    //memcpy(&data, &bufferData+byteOffset, byteLength);
    for (int i = 0; i < byteLength; i++){
        data[i] = bufferData[i+byteOffset];
    }
    
    // write slice to file
    writeBufferFile(data, byteLength, elementType, componentType, bufferId);
}

void GLTFParser::handleBufferInterleaved(
        const tinygltf::Buffer& buffer, 
        int byteOffset, 
        int byteLength, 
        int byteStride,
        int bytesPerElement, 
        int elementType, 
        int componentType,
        int bufferId){
    std::vector<unsigned char> bufferData = buffer.data;
    std::vector<unsigned char> data;
    std::cout << "BufferInterleaved:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  byteLength " << byteLength << std::endl;
    std::cout << "  byteStride " << byteStride << std::endl;
    std::cout << "  bytesPerElement " << bytesPerElement << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bufferId " << bufferId << std::endl;
    std::cout << "" << std::endl;
    // iterate over buffer, one stride at a time 
    for (int i = byteOffset; i < byteOffset + byteLength; i+= byteStride){
        // grab neccessary bytes from stride (byte-by byte, may be slow)
        for (int b = 0; b < bytesPerElement; b++){
            data.push_back(bufferData[i+b]);
        }
    }

    // write data to file
    writeBufferFile(data.data(), byteLength, elementType, componentType, bufferId);
}

void GLTFParser::handleBufferView(
        const tinygltf::BufferView& bufferView, 
        int byteOffset,
        int bytesPerElement, 
        int elementCount, 
        int elementType, 
        int componentType,
        int bufferId){
    std::cout << "BufferView:" << std::endl;
    std::cout << "  byteOffset " << byteOffset << std::endl;
    std::cout << "  bytesPerElement " << bytesPerElement << std::endl;
    std::cout << "  elementCount " << elementCount << std::endl;
    std::cout << "  elementType " << elementType << std::endl;
    std::cout << "  componentType " << componentType << std::endl;
    std::cout << "  bufferId " << bufferId << std::endl;
    // properties
    int adjustedByteOffset = bufferView.byteOffset + byteOffset;
    int byteLength = bufferView.byteLength;
    int byteStride = bufferView.byteStride;
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
        handleBufferInterleaved(buffer, byteOffset, byteStride*elementCount, byteStride, bytesPerElement, elementType, componentType, bufferId);
    
}

int GLTFParser::handleAccessor(const tinygltf::Accessor& accessor){
    int accessorId = m_id++;

    // properties
    int byteOffset = accessor.byteOffset;
    int elementCount = accessor.count;
    int elementType = accessor.type;
    int componentType = accessor.componentType;
    int bytesPerElement = tinygltf::GetNumComponentsInType(elementType) * tinygltf::GetComponentSizeInBytes(componentType);
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

int GLTFParser::handleTexture(const tinygltf::Texture& tex){
    int texId = m_id++;
    
    // get image and sampler
    int sourceIndex = tex.source;
    int samplerIndex = tex.sampler;
    int minFilter;
    tinygltf::Image image;
    tinygltf::Sampler sampler;
    std::cout << "Texture:" << std::endl;
    std::cout << "  sourceIndex " << sourceIndex << std::endl;
    std::cout << "  samplerIndex " << samplerIndex << std::endl;
    if (sourceIndex == -1)
        return texId;
    if (samplerIndex == -1)
        minFilter = TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
    else {
        sampler = model.samplers[samplerIndex];
        minFilter = sampler.minFilter;
    }
    image = model.images[sourceIndex];

    // get min filter
    minFilter = sampler.minFilter;

    // get data
    int elementType = TINYGLTF_TYPE_SCALAR;
    int componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    int bufferId = m_id++;
    if (image.bufferView >= 0){ // bufferview
        int elementCount = image.width * image.height * image.component;
        std::cout << "  [bufferview] " << std::endl;
        std::cout << "  minFilter " << minFilter << std::endl;
        std::cout << "  elementType " << elementType << std::endl;
        std::cout << "  componentType " << componentType << std::endl;
        std::cout << "  elementCount " << elementCount << std::endl;
        handleBufferView(model.bufferViews[image.bufferView], 0, 1, elementCount, elementType, componentType, bufferId);
    } else { // direct buffer
        tinygltf::Buffer buffer;
        int elementCount = image.image.size();
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

    return texId;
}

int GLTFParser::handleTexture(const tinygltf::TextureInfo& texInfo){
    // get texture
    int texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex);
}

int GLTFParser::handleTexture(const tinygltf::NormalTextureInfo& texInfo){
    // get texture
    int texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex);
}

int GLTFParser::handleTexture(const tinygltf::OcclusionTextureInfo& texInfo){
    // get texture
    int texIndex = texInfo.index;
    const tinygltf::Texture& tex = model.textures[texIndex];

    // handle texture
    return handleTexture(tex);
}

int GLTFParser::handleMaterial(const tinygltf::Material& material){
    int materialId = m_id++;
    std::cout << "Material:" << std::endl;
    // material flags
    int materialFlags = 0;

    // tex ids
    std::vector<int> texIds;

    // pbrmetallicroughness
    const tinygltf::PbrMetallicRoughness& pbr = material.pbrMetallicRoughness;
    if (pbr.baseColorTexture.index >= 0){ // base color
        materialFlags |= 0b1;
        std::cout << "  basecolor =========================" << std::endl;
        texIds.push_back(handleTexture(pbr.baseColorTexture));
    }
    if (pbr.metallicRoughnessTexture.index >= 0){ //metallicroughness
        materialFlags |= (0b1<<1);
        std::cout << "  metrough =========================" << std::endl;
        texIds.push_back(handleTexture(pbr.metallicRoughnessTexture));
    }
    // normal
    const tinygltf::NormalTextureInfo& normal = material.normalTexture;
    if (normal.index >= 0){
        materialFlags |= (0b1<<2);
        std::cout << "  normal =========================" << std::endl;
        texIds.push_back(handleTexture(normal));
    }
    // occlusion
    const tinygltf::OcclusionTextureInfo& occlusion = material.occlusionTexture;
    if (occlusion.index >= 0){
        materialFlags |= (0b1<<3);
        std::cout << "  occlusion =========================" << std::endl;
        texIds.push_back(handleTexture(occlusion));
    }
    // emissive
    const tinygltf::TextureInfo& emissive = material.emissiveTexture;
    if (emissive.index >= 0){
        materialFlags |= (0b1<<4);
        std::cout << "  emissive =========================" << std::endl;
        texIds.push_back(handleTexture(emissive));
    }
    // alphamode
    std::string alpha = material.alphaMode;
    if (alpha != "OPAQUE"){
        std::cout << "  alpha =========================" << std::endl;
        materialFlags |= (0b1<<5);
    }
    // doublesided 
    if (material.doubleSided){
        std::cout << "  doublesided =========================" << std::endl;
        materialFlags |= (0b1<<6);
    }
    //TODO: check for specularglossiness + update flags
    std::cout << "" << std::endl;
    // write material to file
    writeMaterialFile(materialFlags, material, texIds, materialId);

    return materialId;
}

int GLTFParser::handlePrimitive(const tinygltf::Primitive& primitive){
    int meshId = m_id++;
    // material
    int materialIndex = primitive.material;

    // attributes (accessor indices)
    int indicesAccessorIndex = primitive.indices;
    int positionAccessorIndex = -1;
    int normalAccessorIndex = -1;
    int tangentAccessorIndex = -1;
    std::vector<int> texcoordAccessorIndices;
    std::vector<int> colorAccessorIndices;

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
    int indicesId = handleAccessor(model.accessors[indicesAccessorIndex]);

    // position
    int positionId = -1;
    if (positionAccessorIndex >= 0){
        positionId = handleAccessor(model.accessors[positionAccessorIndex]);
    }

    // normal
    int normalId = -1;
    if (normalAccessorIndex >= 0){
        normalId = handleAccessor(model.accessors[normalAccessorIndex]);
    }

    // tangent
    int tangentId = -1;
    if (tangentAccessorIndex >= 0){
        tangentId = handleAccessor(model.accessors[tangentAccessorIndex]);
    }

    // texcoords
    std::vector<int> texCoordIds;
    for (auto & texcoordAccessorIndex : texcoordAccessorIndices) {
        texCoordIds.push_back(handleAccessor(model.accessors[texcoordAccessorIndex]));
    }

    // colors
    int colorId = -1;
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
    int materialId = handleMaterial(model.materials[materialIndex]);

    // write prim (mesh) to file
    writeMeshFile(materialId, indicesId, positionId, normalId, colorId, tangentId, texCoordIds, meshId);

    return meshId;
}

void GLTFParser::handleMesh(const tinygltf::Mesh& mesh, std::vector<int> &meshIds){
    for (int i = 0; i < mesh.primitives.size(); i++){
        const tinygltf::Primitive& primitive = mesh.primitives[i];
        if (primitive.mode == 4 || primitive.mode == -1){
            meshIds.push_back(handlePrimitive(primitive));
        }
    }
}

void GLTFParser::parseNode(const tinygltf::Node& node, std::vector<int> &meshIds){
    // handle meshes
    if (node.mesh != -1){
        const tinygltf::Mesh& mesh = model.meshes[node.mesh];
        handleMesh(mesh, meshIds);
    }

    // handle children
    for (int i = 0; i < node.children.size(); i++){
        const tinygltf::Node& child = model.nodes[node.children[i]];
        parseNode(child, meshIds);
    }
}

void GLTFParser::parse(){
    std::cout << "Sucessfully loaded, parsing..." << std::endl;

    int modelId = m_id++;
    // assume one scene
    const tinygltf::Scene& scene = model.scenes[0];
    std::vector<int> meshIds;
    // process top level nodes
    for (int i = 0; i < scene.nodes.size(); i++){
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