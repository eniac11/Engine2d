#include "engine/graphics/Texture.h"
#include <GL/glew.h>

#include <print>
#include <format>
#include <utility>

#include "engine_p/logging_categories.h"

namespace helpers {

    struct compressedTexFeatures {
        bool astc_ldr : 1;
        bool astc_hdr : 1;
        bool bc6h : 1;
        bool bc7 : 1;
        bool etc1 : 1;
        bool etc2 : 1;
        bool bc3 : 1;
        bool pvrtc1 : 1;
        bool pvrtc_srgb : 1;
        bool pvrtc2 : 1;
        bool rgtc : 1;
    };

    compressedTexFeatures determineFeatures() {
        compressedTexFeatures features{};
        GLint num_features;
        GLenum compressed_textures;
        glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &num_features);
        auto formats = new GLint[num_features];
        glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);
        for (ktx_int32_t i = 0; i < num_features; i++) {
            if (formats[i] == GL_COMPRESSED_RGBA8_ETC2_EAC)
                features.etc2 = true;
            if (formats[i] == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
                features.bc3 = true;
            if (formats[i] == GL_COMPRESSED_RG_RGTC2)
                features.rgtc = true;
            if (formats[i] == GL_COMPRESSED_RGBA_ASTC_4x4_KHR)
                features.astc_ldr = true;
            if (formats[i] == GL_COMPRESSED_RGBA_BPTC_UNORM)
                features.bc7 = true;
            if (formats[i] == GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT)
                features.bc6h = true;
        }
        delete[] formats;
        if (num_features < 1) {
            throw texture_exception("No compressed texture support");
        }

        return features;

    }

    void try_transcode_ktxTexture(ktxTexture* texture) {
        if (texture->classId == ktxTexture2_c) {
            if (ktxTexture_NeedsTranscoding(texture)) {
                ktx_transcode_fmt_e tf;

                compressedTexFeatures deviceFeatures = determineFeatures();
                if (deviceFeatures.astc_ldr)
                    tf = KTX_TTF_ASTC_4x4_RGBA;
                else if (deviceFeatures.bc3)
                    tf = KTX_TTF_BC1_OR_3;
                else if (deviceFeatures.etc2)
                    tf = KTX_TTF_ETC; // Let transcoder decide RGB or RGBA
                else if (deviceFeatures.pvrtc1)
                    tf = KTX_TTF_PVRTC1_4_RGBA;
                else if (deviceFeatures.etc1)
                    tf = KTX_TTF_ETC1_RGB;
                else {
                    throw texture_exception("OpenGL implementation does not support any available transcode target.");
                }
                // khr_df_model_e colorModel = ktxTexture2_GetColorModel_e((ktxTexture2*)texture);
                // if (colorModel == KHR_DF_MODEL_UASTC && deviceFeatures.astc_ldr) {
                //     tf = KTX_TTF_ASTC_4x4_RGBA;
                // } else if (colorModel == KHR_DF_MODEL_ETC1S && deviceFeatures.etc2) {
                //     tf = KTX_TTF_ETC;
                // }

                KTX_error_code ktxresult = ktxTexture2_TranscodeBasis((ktxTexture2 *) texture, tf, 0);
                if (KTX_SUCCESS != ktxresult) {

                    throw std::runtime_error(std::format("Transcoding of ktxTexture2 to {} failed: {}", ktxTranscodeFormatString(tf), ktxErrorString(ktxresult)));
                }
            }
        }
    }
}

ktxTexture* read_texture_from_file(std::filesystem::path const& path) {
    if (!std::filesystem::exists(path)) {
        throw texture_exception(std::format("OpenGLTexture::create_texture_from_file: \"{}\" not found", path.string()));
    }
    ktxTexture *ktx_texture;
    KTX_error_code ret = ktxTexture_CreateFromNamedFile(path.c_str(), 0, &ktx_texture);
    // qoi_desc desc;
    // void* data = qoi_read(filepath.c_str(), &desc, channels);
    if (ret == KTX_FILE_OPEN_FAILED) {
        throw texture_exception("Failed to load texture from file: " + path.string());
    }
    if (ret == KTX_INVALID_VALUE) {
        throw texture_exception("Failed to create texture from file: " + path.string() + ". filepath was null");
    }
    if (ret != KTX_SUCCESS) {
        throw texture_exception("Failed to load texture from file");
    }
    helpers::try_transcode_ktxTexture(ktx_texture);

    return ktx_texture;
}

ktxTexture* read_texture_from_file(vfspp::IFilePtr file) {
    std::vector<std::uint8_t> texture_file_buffer;
    size_t size = file->Size();
    file->Read(texture_file_buffer, size);


    ktxTexture *ktx_texture;
    constexpr ktxTextureCreateFlags flags = KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT;
    KTX_error_code ret = ktxTexture_CreateFromMemory(texture_file_buffer.data(), size, flags, &ktx_texture);
    // qoi_desc desc;
    // void* data = qoi_read(filepath.c_str(), &desc, channels);
    if (ret == KTX_FILE_OPEN_FAILED) {
        throw texture_exception("Failed to load texture from file: " + file->GetFileInfo().VirtualPath());
    }
    if (ret == KTX_INVALID_VALUE) {
        throw texture_exception("Failed to create texture from file: " + file->GetFileInfo().VirtualPath() + ". filepath was null");
    }
    if (ret != KTX_SUCCESS) {
        throw texture_exception("Failed to load texture from file");
    }
    helpers::try_transcode_ktxTexture(ktx_texture);

    return ktx_texture;
}
