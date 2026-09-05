#pragma once
#include <cassert>
#include <memory>

#include "Buffer.h"

enum class BufferType : GLenum {
    Uniform_Buffer = GL_UNIFORM_BUFFER,
    Shader_Storage_Buffer = GL_SHADER_STORAGE_BUFFER,
};

template<typename T, BufferType BT>
class StorageBuffer {
    std::shared_ptr<Buffer> m_buffer = nullptr;
    size_t ssize;
    int m_binding_index;

    public:
        StorageBuffer(const int binding_index, graphics::GraphicsBackend* backend) :
                                                                       m_binding_index(binding_index) {
            // ssize = buffer->size();
            m_buffer = backend->create_buffer();
            ssize = sizeof(T);
            m_buffer->allocate(ssize, GL_DYNAMIC_DRAW);

        }

        StorageBuffer(const int binding_index, graphics::GraphicsBackend* backend, size_t n_array) :
                                                                       m_binding_index(binding_index) {
            // ssize = buffer->size();
            m_buffer = backend->create_buffer();
            ssize = sizeof(T) * n_array;
            m_buffer->allocate(ssize, GL_DYNAMIC_DRAW);

        }

        // explicit UnboundBuffer(const int binding_index) : m_buffer(Buffer::create()), m_binding_index(binding_index) {
        //     ssize = sizeof(T);
        //     m_buffer->allocate(ssize, GL_DYNAMIC_DRAW);
        // }

        [[nodiscard]] std::uint32_t get_id() const {
            return m_buffer->get_id();
        }

        void update(T data) {
            m_buffer->upload_subdata(ssize, 0, &data);
        }

        void update(std::span<T> const& data) {
            assert(data.size() <= ssize && "UBO update will overrun the buffer");
            m_buffer->upload_subdata(0, data);
        }

        template<typename TValue>
        void update(const size_t offset, TValue data) {
            constexpr size_t size = sizeof(TValue);

            assert( size+offset <= ssize && "UBO update will overrun the buffer");
            m_buffer->upload_subdata(size, offset, &data);
        }
        template<typename TValue>
        void update(const size_t offset, std::span<TValue> data) {
            constexpr size_t size = sizeof(TValue);

            assert( size+offset <= ssize && "UBO update will overrun the buffer");
            m_buffer->upload_subdata(offset, data);
        }

        void bind() {
            glBindBufferBase(static_cast<GLenum>(BT), m_binding_index, m_buffer->get_id());
        }

        void unbind() {
            glBindBufferBase(static_cast<GLenum>(BT), m_binding_index, 0);
        }

        std::shared_ptr<Buffer> buffer() {
            return m_buffer;
        }

        std::uint32_t binding_index() const {
            return m_binding_index;
        }
};

template<typename T>
using UBO = StorageBuffer<T, BufferType::Uniform_Buffer>;

template<typename T>
using SSBO = StorageBuffer<T, BufferType::Shader_Storage_Buffer>;