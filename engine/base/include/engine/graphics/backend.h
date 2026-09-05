#pragma once
#include <memory>
#include <engine/graphics/Buffer.h>
#include <engine/graphics/VertexArray.h>


#include <engine/graphics/Texture.h>

#include <engine/ShaderProgram.h>
#include <vfspp/IFile.h>

#include <cstddef>

typedef void (*ENGINE_PFNGLGETPROCADDRESS)(const char* proc);

namespace graphics {
    class graphics_backend_exception : public std::runtime_error {
        public:
            explicit graphics_backend_exception(const std::string& basic_string)
                : runtime_error(basic_string) {
            }
    };

    enum struct PrimitiveType {
        POINTS,
        LINE_STRIP,
        TRIANGLE_STRIP,
        TRIANGLES
    };

    enum struct IndexType {
        UNSIGNED_INT,
        UNSIGNED_SHORT,
        UNSIGNED_BYTE
    };

    constexpr std::string to_string(IndexType const e) {
        switch (e) {
            case IndexType::UNSIGNED_INT: return "UNSIGNED_INT";
            case IndexType::UNSIGNED_SHORT: return "UNSIGNED_SHORT";
            case IndexType::UNSIGNED_BYTE: return "UNSIGNED_BYTE";
            default: return "unknown";
        }
    }

    constexpr std::string to_string(PrimitiveType const e) {
        switch (e) {
            case PrimitiveType::POINTS: return "POINTS";
            case PrimitiveType::LINE_STRIP: return "LINE_STRIP";
            case PrimitiveType::TRIANGLE_STRIP: return "TRIANGLE_STRIP";
            case PrimitiveType::TRIANGLES: return "TRIANGLES";
            default: return "unknown";
        }
    }

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
            virtual std::shared_ptr<Texture> create_raw_texture_from_memory(
                const void* data, const int width, const int height, const int channels) = 0;

            // FIXME: Rework this entire api to use a declarative model and not an imperative one.
            virtual void draw(PrimitiveType type, int32_t first, ssize_t count) = 0;
            virtual void draw_elements(PrimitiveType type, ssize_t count, IndexType index_type,
                                       ssize_t first_index) = 0;
            virtual void draw_elements_instanced(::graphics::PrimitiveType type, std::int32_t count,
                                                 ::graphics::IndexType index_type, std::int32_t first_index,
                                                 std::int32_t instances) = 0;

            // Create Shader program


            // std::shared_ptr<ShaderProgram> create_program(ShaderHandle handle) {
            //     return create_shader_program();
            // };

            virtual std::shared_ptr<ShaderProgram> load_program_from_spirv(ShaderHandle handle) = 0;
            virtual std::shared_ptr<ShaderProgram> load_program_from_spirv_memory(
                ShaderHandle handle, std::vector<uint8_t> const& memory) = 0;
            virtual void begin_frame(float dt) = 0;
            virtual void end_frame() = 0;

            // protected:
            //     // TODO: figure out signature and implement
            //     virtual std::shared_ptr<ShaderProgram> create_shader_program() = 0;
    };
} // namespace backend
