
#pragma once
#include <engine/graphics/VertexArray.h>

class OpenGLVertexArray final : public VertexArray {
    struct Private { explicit Private() = default;};
    public:
        OpenGLVertexArray(GLuint id, Private);
        void set_name(std::string const& name) override;

        static std::shared_ptr<OpenGLVertexArray> create();
        int get_id() override { return m_id; };
        void set_ebo(std::shared_ptr<Buffer> ebo) override;
        std::weak_ptr<BufferHandle> add_vertex_data_buffer(std::shared_ptr<Buffer> buffer, GLsizei stride,
            GLuint offset) override;
        void attrib_format(GLint ncomponents, GLuint index, GLenum type,
                           bool normalised, GLuint relative_offset) override;
        void attrib_divisor(std::shared_ptr<BufferHandle> handle, GLuint divisor) override;
        void attrib_format(const Resource& resource, GLenum type,
                           bool normalised, GLuint relative_offset) override;
        void bind_buffer_to_attrib(std::shared_ptr<BufferHandle> handle, GLuint attrib_index) override;
        ~OpenGLVertexArray() override;

    private:
        int find_binding_index() const;

    public:
        std::unique_ptr<memo_type> bind() const override;


    private:
        GLuint m_id;
        BufferHandle m_ebo;
        std::vector<std::shared_ptr<BufferHandle>> m_buffers;
        bool m_deleted = false;
};
