#include "OpenGLBuffer.h"
#include "opengl_logging.h"
#include <engine/elog.h>

std::shared_ptr<OpenGLBuffer> OpenGLBuffer::create() {
    GLuint id;
    glCreateBuffers(1, &id);
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating buffer: {}", id);
    return std::shared_ptr<OpenGLBuffer>(new OpenGLBuffer(id));
}

void OpenGLBuffer::set_name(std::string const& name) { glObjectLabel(GL_BUFFER, get_id(), -1, name.c_str()); }
void OpenGLBuffer::set_name(std::string_view name) { glObjectLabel(GL_BUFFER, get_id(), name.size(), name.data()); }

std::vector<std::shared_ptr<OpenGLBuffer>> OpenGLBuffer::create(const int n) {
    std::vector<std::shared_ptr<OpenGLBuffer>> buffers;
    buffers.reserve(n);
    GLuint buffer_ids[n];
    glCreateBuffers(n, buffer_ids);
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating buffers: {}",
                                                     std::span{buffer_ids, static_cast<unsigned long>(n)});
    for (int i = 0; i < n; i++) {
        buffers.push_back(std::shared_ptr<OpenGLBuffer>(new OpenGLBuffer(buffer_ids[i])));
    }
    return buffers;
}

OpenGLBuffer::~OpenGLBuffer() {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Destroying buffer: {}", m_id);
    glDeleteBuffers(1, &m_id);
    deleted = true;
}


void OpenGLBuffer::allocate(std::size_t size, GLenum flags) {
    std::println(elogCDebug(lcBackendOpengl), "Allocating Buffer({}): size {}", m_id, size);
    glNamedBufferData(m_id, static_cast<GLsizeiptr>(size), nullptr, flags);
}

// TODO(hadley): Determine if this member function can be made const as technically no modifications are made to
//               function arguments or to member variables however glNamedBufferStorage technically
//               modifies memory of GPU and/or CPU.
void OpenGLBuffer::upload(const std::size_t size, const void* data, const GLenum usage) {
    std::println(elogCDebug(lcBackendOpengl), "Upload to Buffer({}): size {}", m_id, size);

    glNamedBufferData(m_id, static_cast<GLsizeiptr>(size), data, usage);
}

void OpenGLBuffer::upload_subdata(std::size_t size, std::size_t offset, const void* data) {
    std::println(elogCDebug(lcBackendOpengl), "Upload to Buffer({}): size {} at offset {}", m_id, size, offset);

    glNamedBufferSubData(m_id, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
}

size_t OpenGLBuffer::size() {
    GLint _size;
    glGetNamedBufferParameteriv(m_id, GL_BUFFER_SIZE, &_size);
    return _size;
}

uint32_t OpenGLBuffer::get_id() const { return m_id; }

OpenGLBuffer::OpenGLBuffer(const GLuint id) : m_id(id) {}
