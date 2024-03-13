// load from a path
#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "ResourceTypes.h"
#include "../../data/asset_ids.h"

namespace spr {
class ResourceLoader {
public:
    ResourceLoader();

    ~ResourceLoader(){}

    template <typename T>
    void loadFromMetadata(ResourceMetadata metadata, T& data);

private:
    // instance id
    uint32 m_instanceId = 0;
    // Model loadModel(ResourceMetadata metadata);
    // Audio loadAudio(ResourceMetadata metadata);
    // Shader loadShader(ResourceMetadata metadata);
    // Texture loadTexture(ResourceMetadata metadata);
};
}