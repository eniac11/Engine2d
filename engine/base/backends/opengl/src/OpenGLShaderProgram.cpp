

#include "OpenGLShaderProgram.h"
#include "opengl_logging.h"

void OpenGLShaderProgram::create_program() {
    m_id = glCreateProgram();
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating Program: {}", m_id);


}

void OpenGLShaderProgram::create_shaders(std::vector<GLenum> const& stages, std::vector<GLuint>& shaders) {
    shaders.reserve(stages.size());
    for (auto const& stage : stages) {
        GLuint shader = glCreateShader(stage);
        shaders.push_back(shader);
    }
    // NOTE(hadley): This only works with libc++
    if (shaders.size() > 1) {
        elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating Shaders: {}", shaders);
    } else if (shaders.size() > 0) {
        elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating Shader: {}", shaders[0]);
    }
    assert(shaders.size() == stages.size());
}

void OpenGLShaderProgram::upload_binary(int nentrypoints, std::vector<GLuint> const& shaders, const unsigned char* binary,
                                  std::size_t binary_size) {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Program Shader Binary (shaders: {0}): size {1}", shaders, binary_size);
    glShaderBinary(nentrypoints, shaders.data(), GL_SHADER_BINARY_FORMAT_SPIR_V, binary, binary_size);
}

void OpenGLShaderProgram::specialise_shaders(std::vector<GLuint> const& shaders,
                                       std::vector<std::string> const& entry_points) {
    for (int i = 0; i < entry_points.size(); i++) {
        std::string const& ep = entry_points[i];
        elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Specialising Shader {0}: '{1}'", shaders[i], ep);
        glSpecializeShader(shaders[i], ep.c_str(), 0, nullptr, nullptr);
    }
}

void OpenGLShaderProgram::attach_shaders(std::vector<GLuint> const& shaders) {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Attaching Shaders {0}: program", shaders, m_id);

    for (int i = 0; i < shaders.size(); i++) {
        glAttachShader(m_id, shaders[i]);
    }
}

void OpenGLShaderProgram::link_program() {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Linking Program: {}",m_id);
    glLinkProgram(m_id);
}

void OpenGLShaderProgram::delete_shaders(std::vector<GLuint> const& shaders) {
    for (int i = 0; i < shaders.size(); i++) {
        glDeleteShader(shaders[i]);
    }
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Deleting Shaders: {}", shaders);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const GLuint& v1)
{
    set_uniform(resource.location, v1);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const GLint& v1)
{
    set_uniform(resource.location, v1);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const GLboolean& v1)
{
    set_uniform(resource.location, v1);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const GLfloat& v1)
{
    set_uniform(resource.location, v1);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const glm::vec3& v1)
{
    set_uniform(resource.location, v1);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const glm::mat4& v1)
{
    set_uniform(resource.location, v1);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const GLuint& v1, const GLuint& v2, const GLuint& v3)
{
    set_uniform(resource.location, v1, v2, v3);
}

void OpenGLShaderProgram::set_uniform(const Resource& resource, const GLfloat& v1, const GLfloat& v2, const GLfloat& v3)
{
    set_uniform(resource.location, v1, v2, v3);
}


struct OpenGLShaderProgramMemo : ShaderProgram::memo_type {
    GLuint id;

    explicit OpenGLShaderProgramMemo(GLuint id)
        : id(id) {
        glUseProgram(id);
    }

    ~OpenGLShaderProgramMemo() override {
        glUseProgram(0);
    };

};


OpenGLShaderProgram::~OpenGLShaderProgram() {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Deleting Program: {}", m_id);
    glDeleteProgram(m_id);
}

std::unique_ptr<ShaderProgram::memo_type> OpenGLShaderProgram::bind() const {
    return std::make_unique<OpenGLShaderProgramMemo>(m_id);
}

std::shared_ptr<OpenGLShaderProgram> OpenGLShaderProgram::create() {
    GLuint const id = glCreateProgram();
    return std::shared_ptr<OpenGLShaderProgram>(new OpenGLShaderProgram(id));
}

GLuint OpenGLShaderProgram::get_id() const {
    return m_id;
}





void OpenGLShaderProgram::add_ubo(int shader_ubo_block_binding, std::shared_ptr<Buffer> buffer) {
    int const binding_index = current_binding_index++;
    glUniformBlockBinding(m_id, shader_ubo_block_binding, binding_index);
    m_ubos.emplace_back(std::move(buffer), shader_ubo_block_binding, binding_index);
}

#include "uniforms.inl"
