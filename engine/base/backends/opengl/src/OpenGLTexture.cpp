
#include "OpenGLTexture.h"
#include "opengl_logging.h"

// OpenGLTexture::OpenGLTexture(const int id, const int width, const int height) : m_id(id), m_width(width), m_height(height) {
// }

OpenGLTexture::OpenGLTexture(GLuint id) : m_id(id) {

}

struct OpenGLTextureMemo : Texture::memo_type {
    GLuint texture_unit;
    GLuint id;

    explicit OpenGLTextureMemo(GLuint id, GLuint m_texture_unit)
        : texture_unit(m_texture_unit), id(id) {
        glBindTextureUnit(texture_unit, id);
    }

    ~OpenGLTextureMemo() override {
        glBindTextureUnit(texture_unit, 0);
    };
};

std::unique_ptr<Texture::memo_type> OpenGLTexture::bind(int binding_index) const {
    return  std::make_unique<OpenGLTextureMemo>(m_id, binding_index);
}

// OpenGLTexture::OpenGLTexture(const int id, ktxTexture *texture) : m_id(id), m_texture(texture) {
//     m_width = texture->baseWidth;
//     m_height = texture->baseHeight;
//     m_channels = texture->baseDepth;
// }

std::shared_ptr<OpenGLTexture> OpenGLTexture::create() {
    GLuint id;
    glGenTextures(1, &id);
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating Texture: {0}", id);
    return std::shared_ptr<OpenGLTexture>(new OpenGLTexture(id));
}

OpenGLTexture::~OpenGLTexture() {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Destroy texture: {0}", m_id);
    glDeleteTextures(1, &m_id);
    if (m_texture != nullptr) {
        ktxTexture_Destroy(m_texture);
    }
}

std::uint32_t OpenGLTexture::get_id() const {
    return m_id;
}

std::uint32_t OpenGLTexture::width() const {
    return m_width;
}

std::uint32_t OpenGLTexture::height() const {
    return m_height;
}

void OpenGLTexture::load_data(const void *data, const int width, const int height, const int channels) {
    m_width = width;
    m_height = height;
    m_channels = channels;
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // // set texture filtering parameters
    glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureStorage2D(m_id, 0, channels == 4 ? GL_RGBA8 : GL_RGB8, width, width);

    glTextureSubImage2D(m_id, 0, 0, 0, m_width, m_height, m_channels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);

    set_valid(true);
}

void OpenGLTexture::load_data(ktxTexture* texture) {
    m_texture = texture;
    m_height = texture->baseHeight;
    m_width = texture->baseWidth;
    m_channels = texture->baseDepth;
    GLenum format;
    GLenum glerror;
    auto ret = ktxTexture_GLUpload(m_texture, &m_id, &format, &glerror);
    if (ret != KTX_SUCCESS) {
        std::string glerror_string;

        throw texture_exception(std::format("Failed to upload texture to opengl: {}", ktxErrorString(ret)));
    }
    set_valid(true);
}

void OpenGLTexture::parameter(TextureParameters const param, int const value) {
    GLuint pname;
    if (param == TextureParameters::MAG_FILTER) {
        pname = GL_TEXTURE_MAG_FILTER;
    } else if (param == TextureParameters::MIN_FILTER) {
        pname = GL_TEXTURE_MIN_FILTER;
    } else {
        throw texture_exception(std::format("Unknown texture parameter: {}", to_string(param)));
    }
    glTextureParameteri(m_id, pname, value);
}


// std::shared_ptr<OpenGLTexture> OpenGLTexture::create_texture_from_file(const std::filesystem::path& filepath) {
//
//     GLuint id;
//     glGenTextures(1, &id);
//     elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating OpenGLTexture: {}", id);
//
//
//
//
//     // ktx_texture->baseWidth;
//     // glCreateTextures(GL_TEXTURE_2D, 1, &id);
//     // glTextureParameteri(id, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
//     // glTextureParameteri(id, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     // // set texture filtering parameters
//     // glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//     // glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     // glTextureStorage2D(id, 1, channels == 4 ? GL_RGBA8 : GL_RGB8, desc.width, desc.height);
//     auto texture = std::make_shared<OpenGLTexture>();
//     // texture.load_data(data);
//     // ::free(data);
//     return texture;
// }

// std::shared_ptr<OpenGLTexture> OpenGLTexture::create_texture_from_memory(const void *data, const int width, const int height, const int channels) {
//
//     // return texture;
// }
