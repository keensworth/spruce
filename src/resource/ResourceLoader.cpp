#include "ResourceLoader.h"

namespace spr{

ResourceLoader::ResourceLoader(){

}

template <typename T>
T ResourceLoader::loadFromMetadata(ResourceMetadata metadata){}


// top-level resources

template <>
Model ResourceLoader::loadFromMetadata<Model>(ResourceMetadata metadata){

}

template <>
Audio ResourceLoader::loadFromMetadata<Audio>(ResourceMetadata metadata){

}

template <>
Shader ResourceLoader::loadFromMetadata<Shader>(ResourceMetadata metadata){

}

template <>
Texture ResourceLoader::loadFromMetadata<Texture>(ResourceMetadata metadata){

}
  
// sub-resources, require reloading top-level parents

template <>
Mesh ResourceLoader::loadFromMetadata<Mesh>(ResourceMetadata metadata){

}

template <>
Material ResourceLoader::loadFromMetadata<Material>(ResourceMetadata metadata){

}

template <>
Buffer ResourceLoader::loadFromMetadata<Buffer>(ResourceMetadata metadata){

}

}