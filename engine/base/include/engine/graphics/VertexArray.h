#pragma once
#include <vector>
#include <memory>
#include <GL/glew.h>
#include "Buffer.h"
#include "engine/ShaderResources.h"
// #include "engine/RAII/VAO_binding.h"

class VertexArray : public std::enable_shared_from_this<VertexArray> {
    public:
        struct BufferHandle {
            std::weak_ptr<VertexArray> vao;
            std::shared_ptr<Buffer> buffer;
            int binding_id;
        };
        struct Memo {
            virtual ~Memo() =default;
        };
        using memo_type = Memo;

        virtual ~VertexArray() = default;

        virtual void set_name(std::string const& name) = 0;


        virtual uint32_t get_id() const = 0;

        // static std::shared_ptr<VertexArray> create();
        virtual void set_ebo(std::shared_ptr<Buffer> ebo) = 0;
        virtual std::weak_ptr<BufferHandle> add_vertex_data_buffer(std::shared_ptr<Buffer> buffer, GLsizei stride,
                                                           GLuint offset = 0) = 0;
        virtual void attrib_format(GLint ncomponents, GLuint attrib_index, GLenum type, bool normalised,
                                   GLuint relative_offset = 0) =0;
    virtual void attrib_format(Resource const& resource, GLenum type,
                               bool normalised, GLuint relative_offset = 0) =0;
    virtual void bind_buffer_to_attrib(std::shared_ptr<BufferHandle> buffer, GLuint attrib_index) = 0;
        virtual void attrib_divisor(std::shared_ptr<BufferHandle> handle, GLuint divisor) = 0;

        virtual std::unique_ptr<memo_type> bind() const = 0;

};
