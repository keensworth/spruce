// load from a path
#pragma once

#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include "ResourceTypes.h"

namespace spr {
class ResourceLoader {
public:
    ResourceLoader();

    ~ResourceLoader(){}

    template <typename T>
    T loadFromMetadata(ResourceMetadata metadata);

private:
    // Model loadModel(ResourceMetadata metadata);
    // Audio loadAudio(ResourceMetadata metadata);
    // Shader loadShader(ResourceMetadata metadata);
    // Texture loadTexture(ResourceMetadata metadata);
};
}