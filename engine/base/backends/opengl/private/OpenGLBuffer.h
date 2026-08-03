
#pragma once
#include <engine/graphics/Buffer.h>
#include <GL/glew.h>


class OpenGLBuffer : public Buffer {
    public:
        static std::shared_ptr<OpenGLBuffer> create();
        static std::vector<std::shared_ptr<OpenGLBuffer>> create(int n);
        ~OpenGLBuffer() override;
        void set_name(std::string const& name) override;
        void set_name(std::string_view name) override;
        void allocate(std::size_t size, GLenum flags) override;
        void upload(std::size_t size, void const* data, GLenum usage) override;
        void upload_subdata(std::size_t size, std::size_t offset, void const* data) override;
        size_t size() override;
        GLuint get_id() const override;
    private:
        explicit OpenGLBuffer(const GLuint id);
        GLuint m_id;
        bool deleted = false;
};
