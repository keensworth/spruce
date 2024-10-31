#include "TextureTranscoder.h"
#include "VulkanDevice.h"
#include "debug/SprLog.h"
#include "ktx.h"
#include "ktxvulkan.h"
#include <cstring>

namespace spr::gfx {

TextureTranscoder::TextureTranscoder(){

}

TextureTranscoder::TextureTranscoder(VulkanDevice* device){
    vkGetPhysicalDeviceFeatures(device->getPhysicalDevice(), &m_features);
    m_device = device;

    m_supported.ETC2 = formatSupported(VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK) || 
                       formatSupported(VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);
    m_supported.BC7 = formatSupported(VK_FORMAT_BC7_UNORM_BLOCK) || 
                      formatSupported(VK_FORMAT_BC7_SRGB_BLOCK);
    m_supported.BC4 = formatSupported(VK_FORMAT_BC4_UNORM_BLOCK) || 
                      formatSupported(VK_FORMAT_BC4_SNORM_BLOCK);
    m_supported.BC3 = formatSupported(VK_FORMAT_BC3_UNORM_BLOCK) || 
                      formatSupported(VK_FORMAT_BC3_SRGB_BLOCK);
    m_supported.BC1 = formatSupported(VK_FORMAT_BC1_RGBA_SRGB_BLOCK) || 
                      formatSupported(VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
    m_supported.ASTC = formatSupported(VK_FORMAT_ASTC_4x4_UNORM_BLOCK) || 
                       formatSupported(VK_FORMAT_ASTC_4x4_SRGB_BLOCK);
    
}

TextureTranscoder::~TextureTranscoder(){

}


void TextureTranscoder::transcode(TranscodeResult& out, VulkanResourceManager* vrm, uint8* data, uint32 size, uint32 width, uint32 height){
    ktxTexture2* texture;
    KTX_error_code result;

    result = ktxTexture2_CreateFromMemory(data, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
    if (result != KTX_SUCCESS){
        SprLog::error("[TextureTranscoder] Failed to load KTX2 texture, code: " + std::string(ktxErrorString(result)));
    }

    // choose optimal transcode target and transcode
    //https://github.com/KhronosGroup/3D-Formats-Guidelines/blob/main/KTXDeveloperGuide.md
    if (ktxTexture2_NeedsTranscoding(texture)){
        ktx_texture_transcode_fmt_e tf;
        khr_df_model_e colorModel = ktxTexture2_GetColorModel_e(texture);

        if (colorModel == KHR_DF_MODEL_UASTC){
            if (m_supported.ASTC) {
                tf = KTX_TTF_ASTC_4x4_RGBA;
            } else if (m_supported.BC7) {
                tf = KTX_TTF_BC7_RGBA;
            } else if (m_supported.ETC2) {
                tf = KTX_TTF_ETC2_RGBA;
            } else if (m_supported.BC3) {
                tf = KTX_TTF_BC3_RGBA;
            } else if (m_supported.BC1) {
                tf = KTX_TTF_BC1_RGB;
            } else {
                tf = KTX_TTF_NOSELECTION;
            }
        } else if (colorModel == KHR_DF_MODEL_ETC1S){
            if (m_supported.ETC2){
                tf = KTX_TTF_ETC2_RGBA;
            } else if (m_supported.BC7) {
                tf = KTX_TTF_BC7_RGBA;
            } else if (m_supported.BC3) {
                tf = KTX_TTF_BC3_RGBA;
            } else if (m_supported.BC1) {
                tf = KTX_TTF_BC1_RGB;
            } else {
                tf = KTX_TTF_NOSELECTION;
            }
        } else {
            SprLog::warn("[TextureTranscoder] Texture needs transcode, but none applicable");
            tf = KTX_TTF_NOSELECTION;
        }

        // transcode to target format
        result = ktxTexture2_TranscodeBasis(texture, tf, 0);
        if (result){
            SprLog::error("[TextureTranscoder] Failed to transcode texture, code: " + std::string(ktxErrorString(result)));
        }
    }

    uint32 mipLevels = texture->numLevels;
    uint32 layers = texture->numFaces;

    VkFormat format = ktxTexture2_GetVkFormat(texture);
    uint32 sizeBytes = ktxTexture_GetDataSize(ktxTexture(texture));
    
    out.format = format;
    out.mips = mipLevels;
    out.layers = layers;
    out.sizeBytes = sizeBytes;

    m_activeTexture = texture;
}

void TextureTranscoder::destroyActiveTexture(TranscodeResult& out, uint32 sizeBytes){
    ktx_uint8_t* transcodedData = ktxTexture_GetData(ktxTexture(m_activeTexture));

    out.transcodedData.allocateAndInsert<ktx_uint8_t>({
        .data = transcodedData,
        .size = sizeBytes
    });

    ktxTexture_Destroy(ktxTexture(m_activeTexture));
}

bool TextureTranscoder::formatSupported(VkFormat format){
    VkFormatProperties properties;
    vkGetPhysicalDeviceFormatProperties(m_device->getPhysicalDevice(), format, &properties);

    return (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
}

void TextureTranscoder::reset(){
    m_deletionQueue.execute();
}

}