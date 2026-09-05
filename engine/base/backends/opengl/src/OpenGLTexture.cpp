#include "OpenGLTexture.h"
#include "opengl_logging.h"

// OpenGLTexture::OpenGLTexture(const int id, const int width, const int height) : m_id(id), m_width(width), m_height(height) {
// }

OpenGLTexture::OpenGLTexture(GLuint id) : m_id(id) {
}

constexpr GLenum texture_target_to_texture_binding_target(GLenum const& texture_target_type) {
    switch (texture_target_type) {
        case GL_TEXTURE_1D:
            return GL_TEXTURE_BINDING_1D;
        case GL_TEXTURE_2D:
            return GL_TEXTURE_BINDING_2D;
        case GL_TEXTURE_3D:
            return GL_TEXTURE_BINDING_3D;
        case GL_TEXTURE_CUBE_MAP:
            return GL_TEXTURE_BINDING_CUBE_MAP;
        case GL_TEXTURE_1D_ARRAY:
            return GL_TEXTURE_BINDING_1D_ARRAY;
        case GL_TEXTURE_2D_ARRAY:
            return GL_TEXTURE_BINDING_2D_ARRAY;
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            return GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
        default:
            return GL_INVALID_ENUM;
    }
}

struct OpenGLTextureMemo : Texture::memo_type {
    GLuint m_texture_unit;
    GLuint m_id;
    GLuint m_previous_texture_id;

    explicit OpenGLTextureMemo(Texture const& tex, GLuint texture_unit)
        : m_texture_unit(texture_unit), m_id(tex.get_id()) {
        // Get the texture target type of the texture
        GLenum target;
        tex.get_parameter(TextureParameters::TEXTURE_TARGET, target);
        // Get the texture id or 0 that is assigned to the currently bound texture unit.
        GLint prev_id;
        glGetIntegeri_v(texture_target_to_texture_binding_target(target), texture_unit, &prev_id);
        // Check for negative value before converting to unsigned
        assert(prev_id >= 0);
        // TODO: Maybe assert that id is less than "max available textures"?
        m_previous_texture_id = prev_id;
        elogCDebugEnabled(lcBackendOpenglBindings)
            std::println(
                elogCDebug(lcBackendOpenglBindings), "Binding Texture({0}) to TextureUnit({1}): previous Texture({2})",
                m_id, m_texture_unit, m_previous_texture_id);
        glBindTextureUnit(texture_unit, m_id);
    }

    ~OpenGLTextureMemo() override {
        if (m_previous_texture_id == 0 or m_previous_texture_id == m_id) {
            elogCDebugEnabled(lcBackendOpenglBindings)
                std::println(
                    elogCDebug(lcBackendOpenglBindings), "Unbinding Texture({0}) from TextureUnit({1})", m_id,
                    m_texture_unit);
            glBindTextureUnit(m_texture_unit, 0);
            return;
        }
        elogCDebugEnabled(lcBackendOpenglBindings)
            std::println(
                elogCDebug(lcBackendOpenglBindings),
                "Rebinding Texture({2}) to TextureUnit({1}): previous Texture({0})", m_id, m_texture_unit,
                m_previous_texture_id);
        glBindTextureUnit(m_texture_unit, m_previous_texture_id);
    };
};

std::unique_ptr<Texture::memo_type> OpenGLTexture::bind(int binding_index) const {
    return std::make_unique<OpenGLTextureMemo>(*this, binding_index);
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

bool OpenGLTexture::has_layers() {
    if (m_texture && m_texture->numLayers > 0) {
        return true;
    }
    return false;
}

std::uint32_t OpenGLTexture::layers() {
    if (m_texture)
        return m_texture->numLayers;
    return 0;
}

void OpenGLTexture::load_data(const void* data, const int width, const int height, const int channels) {
    m_width = width;
    m_height = height;
    m_channels = channels;
    // glTextureParameteri(m_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // set texture wrapping to GL_REPEAT (default wrapping method)
    // glTextureParameteri(m_id, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // // set texture filtering parameters
    // glTextureParameteri(m_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTextureParameteri(m_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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

void OpenGLTexture::get_parameter(TextureParameters param, int& value) const {
    GLenum pname;
    if (param == TextureParameters::MAG_FILTER) {
        pname = GL_TEXTURE_MAG_FILTER;
    } else if (param == TextureParameters::MIN_FILTER) {
        pname = GL_TEXTURE_MIN_FILTER;
    } else {
        throw texture_exception(std::format("Unknown texture parameter: {}", to_string(param)));
    }
    glGetTextureParameteriv(m_id, pname, &value);
}

void OpenGLTexture::get_parameter(TextureParameters param, std::uint32_t& value) const {
    GLenum pname;
    if (param == TextureParameters::TEXTURE_TARGET) {
        pname = GL_TEXTURE_TARGET;
    } else {
        throw texture_exception(std::format("Unknown texture parameter: {}", to_string(param)));
    }
    glGetTextureParameterIuiv(m_id, pname, &value);
}

void OpenGLTexture::set_parameter(TextureParameters const param, int const value) {
    GLenum pname;
    if (param == TextureParameters::MAG_FILTER) {
        pname = GL_TEXTURE_MAG_FILTER;
    } else if (param == TextureParameters::MIN_FILTER) {
        pname = GL_TEXTURE_MIN_FILTER;
    } else if (param == TextureParameters::WRAP_S) {
        pname = GL_TEXTURE_WRAP_S;
    } else if (param == TextureParameters::WRAP_T) {
        pname = GL_TEXTURE_WRAP_T;
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
