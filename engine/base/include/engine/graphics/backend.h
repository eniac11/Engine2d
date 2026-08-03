#pragma once
#include <memory>
#include <engine/graphics/Buffer.h>
#include <engine/graphics/VertexArray.h>


#include <engine/graphics/Texture.h>

#include <engine/ShaderProgram.h>
#include <vfspp/IFile.h>

typedef void (*ENGINE_PFNGLGETPROCADDRESS)(const char* proc);

namespace graphics {
    class graphics_backend_exception : public std::runtime_error {
        public:
            explicit graphics_backend_exception(const std::string& basic_string)
                : runtime_error(basic_string) {
            }
    };

    class GraphicsBackend {
        // TODO(hadley): add "object label/name" overrides
        public:
            virtual ~GraphicsBackend() = default;
            /**
             * @throw graphics::graphics_backend_exception if initialising the backend fails
             */
            virtual void initialise(ENGINE_PFNGLGETPROCADDRESS* pfngetprocaddress) = 0;
            virtual void deinitialise() = 0;

            // Create Buffers
            virtual std::shared_ptr<Buffer> create_buffer() = 0;
            virtual std::vector<std::shared_ptr<Buffer>> create_buffer(int n) = 0;

            // Create Vertex Array
            virtual std::shared_ptr<VertexArray> create_vao(std::string name) =0;
            virtual std::shared_ptr<VertexArray> create_vao() =0;

            virtual std::vector<std::shared_ptr<VertexArray>> create_vao(int n) =0;

            // Create Textures
            virtual std::shared_ptr<Texture> create_texture() = 0;
            virtual std::shared_ptr<Texture> create_texture_from_file(std::filesystem::path const& path) = 0;
            virtual std::shared_ptr<Texture> create_texture_from_file(vfspp::IFilePtr file) = 0;
            virtual std::shared_ptr<Texture> create_raw_texture_from_memory(const void *data, const int width, const int height, const int channels) = 0;

            // Create Shader program


            // std::shared_ptr<ShaderProgram> create_program(ShaderHandle handle) {
            //     return create_shader_program();
            // };

            virtual std::shared_ptr<ShaderProgram> load_program_from_spirv(ShaderHandle handle) = 0;
            virtual std::shared_ptr<ShaderProgram> load_program_from_spirv_memory(ShaderHandle handle, std::vector<uint8_t> const& memory) = 0;

        // protected:
        //     // TODO: figure out signature and implement
        //     virtual std::shared_ptr<ShaderProgram> create_shader_program() = 0;
    };
} // namespace backend
