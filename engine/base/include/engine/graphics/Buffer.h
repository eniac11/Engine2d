#pragma once
#include <GL/glew.h>
#include <memory>
#include <string_view>
#include <vector>
#include <span>

class Buffer {
    public:
        virtual ~Buffer() = default;

        virtual void set_name(std::string const& name) = 0;
        virtual void set_name(std::string_view name) = 0;
        /**
         * Allocate (initialise) a gl buffer with size
         * @param size size of the gl buffer to allocate (data is nullptr)
         * @param flags GL_STATIC_DRAW | GL_DYNAMIC_DRAW | etc..
         * @remark Use this when uploading subdata.
         */
        virtual void allocate(std::size_t size, GLenum flags) = 0;
        virtual void upload(std::size_t size, const void* data, GLenum usage) = 0;
        template<typename T>
        void upload(std::span<T> const& buffer , GLenum usage) {
            this->upload(buffer.size_bytes(), buffer.data(), usage);
        }
        virtual void upload_subdata(std::size_t size, std::size_t offset, const void* data) = 0;
        template <typename T>
        void upload_subdata(std::size_t offset, std::span<T> const& buffer) {
            this->upload_subdata(buffer.size_bytes(), offset, buffer.data());
        }
        virtual size_t size() = 0;

        [[nodiscard]] virtual uint32_t get_id() const = 0;
};
