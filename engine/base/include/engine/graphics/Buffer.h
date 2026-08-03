#pragma once
#include <GL/glew.h>
#include <memory>
#include <string_view>
#include <vector>

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
        virtual void upload_subdata(std::size_t size, std::size_t offset, const void* data) = 0;
        virtual size_t size() = 0;

        virtual GLuint get_id() const = 0;
};
