#pragma once
#include <engine/graphics/backend.h>

namespace backend::graphics {
    class OpenglBackend final : public ::graphics::GraphicsBackend {
        public:
            ~OpenglBackend() override;
            void initialise(ENGINE_PFNGLGETPROCADDRESS* pfngetprocaddress) override;
            void deinitialise() override;

            void begin_frame(float dt) override;
            void end_frame() override;

            std::shared_ptr<Buffer> create_buffer() override;
            std::vector<std::shared_ptr<Buffer>> create_buffer(int n) override;
            std::shared_ptr<VertexArray> create_vao() override;
            std::shared_ptr<VertexArray> create_vao(std::string name) override;
            std::vector<std::shared_ptr<VertexArray>> create_vao(int n) override;
            std::shared_ptr<Texture> create_texture() override;
            std::shared_ptr<Texture> create_texture_from_file(std::filesystem::path const& path) override;
            std::shared_ptr<Texture> create_texture_from_file(vfspp::IFilePtr file) override;
            std::shared_ptr<Texture> create_raw_texture_from_memory(const void* data, const int width, const int height,
                                                                    const int channels) override;
            std::shared_ptr<ShaderProgram> load_program_from_spirv(ShaderHandle handle) override;
            std::shared_ptr<ShaderProgram> load_program_from_spirv_memory(
                ShaderHandle handle, std::vector<uint8_t> const& memory) override;

            void draw(::graphics::PrimitiveType type, int32_t first, ssize_t count) override;
            void draw_elements(::graphics::PrimitiveType type, ssize_t count,
                               ::graphics::IndexType index_type, ssize_t first_index) override;
            void draw_elements_instanced(::graphics::PrimitiveType type, std::int32_t count,
                                         ::graphics::IndexType index_type, std::int32_t first_index,
                                         std::int32_t instances) override;

        private:
            void setup_from_env();
            bool m_enable_validation = false;
    };
}
