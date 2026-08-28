
#pragma once
#include <engine/ShaderProgram.h>


class OpenGLShaderProgram final : public ShaderProgram {
    public:
        static std::shared_ptr<OpenGLShaderProgram> create();
        uint32_t get_id() const override;
        void add_ubo(int shader_ubo_block_binding, std::shared_ptr<Buffer> buffer) override;

    public:
        void create_program();
        void create_shaders(std::vector<GLenum> const& stages, std::vector<GLuint>& shaders);
        void upload_binary(int nentrypoints, std::vector<GLuint> const& shaders, unsigned char const* binary,
            std::size_t binary_size);
        void specialise_shaders(std::vector<GLuint> const& shaders,
            std::vector<std::string> const& entry_points);
        void attach_shaders(std::vector<GLuint> const& shaders);
        void link_program();
        void delete_shaders(std::vector<GLuint> const& shaders);

        void set_uniform(GLint location, const GLuint& v1) override;
        void set_uniform(GLint location, const GLint& v1) override;
        void set_uniform(GLint location, const GLboolean& v1) override;
        void set_uniform(GLint location, const GLfloat& v1) override;
        void set_uniform(GLint location, const glm::vec3& v1) override;
        void set_uniform(GLint location, const glm::mat4& v1) override;

        void set_uniform(GLint location, const GLuint& v1, const GLuint& v2, const GLuint& v3) override;
        void set_uniform(GLint location, const GLfloat& v1, const GLfloat& v2, const GLfloat& v3) override;

        void set_uniform(const Resource& resource, const GLuint& v1) override;
        void set_uniform(const Resource& resource, const GLint& v1) override;
        void set_uniform(const Resource& resource, const GLboolean& v1) override;
        void set_uniform(const Resource& resource, const GLfloat& v1) override;
        void set_uniform(const Resource& resource, const glm::vec3& v1) override;
        void set_uniform(const Resource& resource, const glm::mat4& v1) override;

        void set_uniform(const Resource& resource, const GLuint& v1, const GLuint& v2, const GLuint& v3) override;
        void set_uniform(const Resource& resource, const GLfloat& v1, const GLfloat& v2, const GLfloat& v3) override;

    private:
        OpenGLShaderProgram(GLuint const id) : m_id(id) {}

    public:
        ~OpenGLShaderProgram() override;
        std::unique_ptr<memo_type> bind() const override;


    private:
        GLuint m_id;
        int current_binding_index = 0;
};
