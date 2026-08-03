#pragma once
#include <memory>
#include <type_traits>

#include "Buffer.h"

enum class BufferType : GLenum {
    Uniform_Buffer = GL_UNIFORM_BUFFER,
    Shader_Storage_Buffer = GL_SHADER_STORAGE_BUFFER,
};

template<typename T, BufferType BT>
class UnboundBuffer {
    std::shared_ptr<Buffer> m_buffer;
    size_t ssize;
    int m_binding_index;

    public:
        UnboundBuffer(const int binding_index, std::shared_ptr<Buffer> buffer) : m_buffer(std::move(buffer)),
                                                                       m_binding_index(binding_index) {
            // ssize = buffer->size();
            ssize = sizeof(T);
            m_buffer->allocate(ssize, GL_DYNAMIC_DRAW);

        }

        // explicit UnboundBuffer(const int binding_index) : m_buffer(Buffer::create()), m_binding_index(binding_index) {
        //     ssize = sizeof(T);
        //     m_buffer->allocate(ssize, GL_DYNAMIC_DRAW);
        // }

        int get_id() const {
            return m_buffer->get_id();
        }

        void update(T data) {
            m_buffer->upload(ssize, &data, GL_DYNAMIC_DRAW);
        }

        template<typename TValue>
        void update(const size_t offset, TValue data) {
            constexpr size_t size = sizeof(TValue);

            assert( size+offset <= ssize && "UBO update will overrun the buffer");
            m_buffer->upload_subdata(size, offset, &data);
        }

        void bind() {
            glBindBufferBase(static_cast<GLenum>(BT), m_binding_index, m_buffer->get_id());
        }

        void unbind() {
            glBindBufferBase(static_cast<GLenum>(BT), m_binding_index, 0);
        }
};

template<typename T>
using UBO = UnboundBuffer<T, BufferType::Uniform_Buffer>;

template<typename T>
using SSBO = UnboundBuffer<T, BufferType::Shader_Storage_Buffer>;