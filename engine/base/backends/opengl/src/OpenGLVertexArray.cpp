

#include "OpenGLVertexArray.h"
#include "opengl_logging.h"
#include <ranges>
#include <algorithm>
#include <cassert>
#include <print>

std::shared_ptr<OpenGLVertexArray> OpenGLVertexArray::create() {
    GLuint id;
    glCreateVertexArrays(1, &id);
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Creating VAO: {}", id);
    return std::make_shared<OpenGLVertexArray>(id, Private());
}

void OpenGLVertexArray::set_ebo(std::shared_ptr<Buffer> ebo) {
    m_ebo = BufferHandle{weak_from_this(), std::move(ebo)};
    glVertexArrayElementBuffer(m_id, m_ebo.buffer->get_id());
}

std::weak_ptr<VertexArray::BufferHandle> OpenGLVertexArray::add_vertex_data_buffer(std::shared_ptr<Buffer> buffer, const GLsizei stride, const GLuint offset) {
    auto handle = std::make_shared<VertexArray::BufferHandle>(weak_from_this(), std::move(buffer));
    m_buffers.push_back(handle);
    handle->binding_id = find_binding_index();
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Binding Buffer({}) to VAO({}): {}", handle->buffer->get_id(), m_id, handle->binding_id);
    // std::println("{}", handle.binding_id);
    glVertexArrayVertexBuffer(m_id, handle->binding_id, handle->buffer->get_id(), offset, stride);
    return handle;
}

void OpenGLVertexArray::attrib_format(GLint ncomponents, GLuint index, GLenum type, bool normalised, GLuint relative_offset) {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "VertexArray({0}) attrib format: attrib({1}), components({2})", m_id, index, ncomponents);
    if (type == GL_UNSIGNED_INT or type == GL_INT or type == GL_UNSIGNED_BYTE ) {
        glVertexArrayAttribIFormat(m_id, index, ncomponents, type, relative_offset);
    } else {
        glVertexArrayAttribFormat(m_id, index, ncomponents, type, normalised, relative_offset);
    }

    glEnableVertexArrayAttrib(m_id, index);
}

void OpenGLVertexArray::attrib_format(const Resource& resource, GLenum type,
                                      bool normalised, GLuint relative_offset)
{
    attrib_format(resource.ncomponents, resource.location, type, normalised, relative_offset);
}

void OpenGLVertexArray::bind_buffer_to_attrib(std::shared_ptr<BufferHandle> handle, GLuint attrib_index)
{
    if (auto vao = handle->vao.lock())
    {
        if (vao->get_id() == m_id)
        {
            elogCDebugEnabled(lcBackendOpenglBindings) std::println(elogCDebug(lcBackendOpenglBindings), "VertexArray({3}) Binding buffer({1}): {0} -> {2}", handle->binding_id, handle->buffer->get_id(), attrib_index, m_id);

            glVertexArrayAttribBinding(m_id, attrib_index, handle->binding_id);

        } else
        {
            throw std::runtime_error("The supplied binding handle was not created by this vao.");
        }
    } else
    {
        throw std::runtime_error("Cannot bind buffer with VAO");
    }
}

void OpenGLVertexArray::attrib_divisor(std::shared_ptr<BufferHandle> handle, GLuint divisor) {
    glVertexArrayBindingDivisor(m_id, handle->binding_id, divisor);
}




OpenGLVertexArray::~OpenGLVertexArray() {
    elogCDebugEnabled(lcBackendOpengl) std::println(elogCDebug(lcBackendOpengl), "Destroying VertexArray: {}", m_id);
    glDeleteVertexArrays(1, &m_id);
    m_deleted = true;

}

OpenGLVertexArray::OpenGLVertexArray(const GLuint id, Private) : m_id(id) {
}

void OpenGLVertexArray::set_name(std::string const& name) {
    glObjectLabel(GL_VERTEX_ARRAY, m_id, name.length(), name.c_str());
}

int OpenGLVertexArray::find_binding_index() const {
    std::vector<int> binding_indexes;
    binding_indexes.reserve(m_buffers.size());
for (auto const& handle: m_buffers) {
        binding_indexes.emplace_back(handle->binding_id);
    }
    std::ranges::sort(binding_indexes);
    // contiguous_index: 1 2 3 4 5
    int contiguous_index = 0;
    for (auto const& current_index: binding_indexes) {
        if (current_index != contiguous_index) {
            // 0 1  2 i3 4 5
            // 0 1  2 c4 5 6
            // return 3
            return contiguous_index;
        }
        contiguous_index++;
    }
    // contiguous_index++;
    return contiguous_index;
}

struct OpenGLVertexArrayMemo : VertexArray::memo_type {
    GLuint m_id;
    GLuint m_previous_id;

    explicit OpenGLVertexArrayMemo(GLuint m_id)
        : m_id(m_id) {
        GLint prev_vao_id;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev_vao_id);
        assert(prev_vao_id >= 0);
        m_previous_id = prev_vao_id;
        elogCDebugEnabled(lcBackendOpenglBindings) std::println(elogCDebug(lcBackendOpenglBindings), "Binding VertexArray({0}): previous VertexArray({1})", m_id, m_previous_id);
        glBindVertexArray(m_id);
    }

    ~OpenGLVertexArrayMemo() override {
        if (m_previous_id == 0 or m_previous_id == m_id) {
            elogCDebugEnabled(lcBackendOpenglBindings) std::println(elogCDebug(lcBackendOpenglBindings), "Unbinding VertexArray({0})", m_id, m_previous_id);

            glBindVertexArray(0);
            return;
        }
        elogCDebugEnabled(lcBackendOpenglBindings) std::println(elogCDebug(lcBackendOpenglBindings), "Rebinding VertexArray({1}): previous VertexArray({0})", m_id, m_previous_id);
        glBindVertexArray(m_previous_id);
    };
};

std::unique_ptr<VertexArray::memo_type> OpenGLVertexArray::bind() const {
    return std::make_unique<OpenGLVertexArrayMemo>(m_id);
}
