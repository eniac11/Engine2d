#include <engine/backends/graphics/opengl_backend.h>
#include <format>
#include <ktx.h>
#include "OpenGLBuffer.h"
#include "OpenGLShaderProgram.h"
#include "OpenGLTexture.h"
#include "OpenGLVertexArray.h"
#include <imgui/imgui.h>

#include "imgui_impl_opengl3.h"

#include "opengl_logging.h"

constexpr std::string debug_source_message(GLenum source) {
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            return "glAPI";
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            return "Shader Compiler";
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            return "Window System";
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            return "Third Party";
        case GL_DEBUG_SOURCE_APPLICATION:
            return "Application";
        case GL_DEBUG_SOURCE_OTHER:
            return "Other";
        default:
            return "Unknown";
    }
}

constexpr std::string debug_type_message(GLenum type) {
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            return "error";
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            return "deprecated";
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            return "undefined behaviour";
        case GL_DEBUG_TYPE_PORTABILITY:
            return "implementation dependent";
        case GL_DEBUG_TYPE_PERFORMANCE:
            return "performance";
        case GL_DEBUG_TYPE_OTHER:
            return "other";
        case GL_DEBUG_TYPE_MARKER:
            return "marker";
        case GL_DEBUG_TYPE_PUSH_GROUP:
            return "begin group";
        case GL_DEBUG_TYPE_POP_GROUP:
            return "end group";
        default:
            return "unknown";
    }
}

// The callback function to handle debug messages
void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                   const GLchar* message, const void* userParam) {
    // if (!(severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_HIGH)) {
    //     return;
    // }
    std::println(elogCDebug(lcBackendOpengl), "[{0}, {1}]: {2}", debug_source_message(source), debug_type_message(type),
                 message);
}

namespace backend::graphics {
    OpenglBackend::~OpenglBackend() {
    }

    void OpenglBackend::initialise(ENGINE_PFNGLGETPROCADDRESS* pfngetprocaddress) {
        setup_from_env();
        if (const GLenum error = glewInit(); error != GLEW_OK) {
            std::string const error_text((char*)glewGetErrorString(error));
            throw ::graphics::graphics_backend_exception(
                std::format("Failed to initialise Opengl Backend: {}", error_text));
        }

        ktxLoadOpenGL((PFNGLGETPROCADDRESS)pfngetprocaddress);

        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);

        // Register callback
        glDebugMessageCallback(debugCallback, nullptr);

        ImGui_ImplOpenGL3_Init(nullptr);
        // if (m_enable_validation)
        // {
        //     glEnableVa
        // }
    }

    void OpenglBackend::deinitialise() {
        ImGui_ImplOpenGL3_Shutdown();
    }

    void OpenglBackend::begin_frame(float dt) {
        ImGui_ImplOpenGL3_NewFrame();
    }

    void OpenglBackend::end_frame() {
        ImDrawData* draw_data = ImGui::GetDrawData();
        ImGui_ImplOpenGL3_RenderDrawData(draw_data);
    }

    std::shared_ptr<Buffer> OpenglBackend::create_buffer() {
        return OpenGLBuffer::create();
    }

    std::vector<std::shared_ptr<Buffer>> OpenglBackend::create_buffer(int n) {
        auto glbuffers = OpenGLBuffer::create(n);
        // std::vector is not covariant there for OpenGLBuffer* is not converted to Buffer*
        // have to create a Buffer* vector and "copy" OpenGLBuffer* to vector Buffer*
        // this operation is covariant.
        std::vector<std::shared_ptr<Buffer>> buffers{};
        // Use "move" instead of "copy" to not needlessly increment the shared_ptr ref count.
        std::ranges::move(glbuffers, std::back_inserter(buffers));
        return buffers;
    }

    std::shared_ptr<VertexArray> OpenglBackend::create_vao() {
        return OpenGLVertexArray::create();
    }

    std::shared_ptr<VertexArray> OpenglBackend::create_vao(std::string name) {
        auto&& vao = create_vao();
        vao->set_name(name);
        return vao;
    }

    std::vector<std::shared_ptr<VertexArray>> OpenglBackend::create_vao(int n) {
    }

    std::shared_ptr<Texture> OpenglBackend::create_texture() {
    }

    std::shared_ptr<Texture> OpenglBackend::create_texture_from_file(std::filesystem::path const& path) {
        ktxTexture* ktx_texture = read_texture_from_file(path);
        auto gltexture = OpenGLTexture::create();
        gltexture->load_data(ktx_texture);
        return gltexture;
    }

    std::shared_ptr<Texture> OpenglBackend::create_texture_from_file(vfspp::IFilePtr file) {
        ktxTexture* ktx_texture = read_texture_from_file(file);
        auto gltexture = OpenGLTexture::create();
        gltexture->load_data(ktx_texture);
        return gltexture;
    }

    std::shared_ptr<Texture> OpenglBackend::
    create_raw_texture_from_memory(const void* data, const int width, const int height, const int channels) {
        auto gltexture = OpenGLTexture::create();
        gltexture->load_data(data, width, height, channels);
        return gltexture;
    }

    std::shared_ptr<ShaderProgram> OpenglBackend::load_program_from_spirv(ShaderHandle const handle) {
        auto program = OpenGLShaderProgram::create();
        // std::shared_ptr<ShaderProgram> program = backend->create_program();
        // std::shared_ptr<ShaderProgram> program(p);
        // program.reset(new ShaderProgram());
        program->create_program();

        std::vector<GLenum> stages;
        std::vector<std::string> entrypoints;
        for (EntryPoint const& entry_point : handle.entry_points()) {
            stages.push_back(entry_point.stage);
            entrypoints.push_back(entry_point.name);
        }
        std::vector<GLuint> shaders;
        program->create_shaders(stages, shaders);

        program->upload_binary(entrypoints.size(), shaders, handle.program(), handle.size());
        program->specialise_shaders(shaders, entrypoints);

        program->attach_shaders(shaders);

        program->link_program();
        program->delete_shaders(shaders);

        // GLuint shaders[handle.entry_points().size()];
        // for (int i = 0; i < handle.entry_points().size(); i++) {
        //     shaders[i] = glCreateShader(handle.entry_points()[i].stage);
        // }
        // glShaderBinary(handle.entry_points().size(), shaders, GL_SHADER_BINARY_FORMAT_SPIR_V, handle.program(),
        //                handle.size());
        // for (int i = 0; i < handle.entry_points().size(); i++) {
        //     glSpecializeShader(shaders[i], handle.entry_points()[i].name.c_str(), 0, nullptr, nullptr);
        // }
        // const GLuint shader_program = glCreateProgram();
        // for (int i = 0; i < handle.entry_points().size(); i++) {
        //     glAttachShader(shader_program, shaders[i]);
        // }
        // glLinkProgram(shader_program);
        // for (int i = 0; i < handle.entry_points().size(); i++) {
        //     glDeleteShader(shaders[i]);
        // }
        return program;
    }

    std::shared_ptr<ShaderProgram> OpenglBackend::load_program_from_spirv_memory(ShaderHandle handle,
        std::vector<uint8_t> const& memory) {
        auto program = OpenGLShaderProgram::create();
        // std::shared_ptr<ShaderProgram> program = backend->create_program();
        // std::shared_ptr<ShaderProgram> program(p);
        // program.reset(new ShaderProgram());
        program->create_program();

        std::vector<GLenum> stages;
        std::vector<std::string> entrypoints;
        for (EntryPoint const& entry_point : handle.entry_points()) {
            stages.push_back(entry_point.stage);
            entrypoints.push_back(entry_point.name);
        }
        std::vector<GLuint> shaders;
        program->create_shaders(stages, shaders);

        program->upload_binary(entrypoints.size(), shaders, memory.data(), memory.size());
        program->specialise_shaders(shaders, entrypoints);

        program->attach_shaders(shaders);

        program->link_program();
        program->delete_shaders(shaders);
        return program;
    }

    constexpr GLenum map_primitive_type(::graphics::PrimitiveType const type) {
        switch (type) {
            case ::graphics::PrimitiveType::POINTS:
                return GL_POINTS;
            case ::graphics::PrimitiveType::LINE_STRIP:
                return GL_LINE_STRIP;
            case ::graphics::PrimitiveType::TRIANGLE_STRIP:
                return GL_TRIANGLE_STRIP;
            case ::graphics::PrimitiveType::TRIANGLES:
                return GL_TRIANGLES;
        }
    }

    constexpr GLenum map_index_type(::graphics::IndexType const type) {
        switch (type) {
            case ::graphics::IndexType::UNSIGNED_INT:
                return GL_UNSIGNED_INT;
            case ::graphics::IndexType::UNSIGNED_SHORT:
                return GL_UNSIGNED_SHORT;
            case ::graphics::IndexType::UNSIGNED_BYTE:
                return GL_UNSIGNED_BYTE;
        }
    }

    void OpenglBackend::draw(::graphics::PrimitiveType type, int32_t first, ssize_t count) {
        GLenum const primitive_type = map_primitive_type(type);
        glDrawArrays(primitive_type, first, count);
    }

    void OpenglBackend::draw_elements(::graphics::PrimitiveType type, ssize_t count,
                                      ::graphics::IndexType index_type, ssize_t first_index) {
        GLenum const primitive_type = map_primitive_type(type);
        GLenum const itype = map_index_type(index_type);
        // NOTE(hadley): Check this for correctness: Does glDrawElements take a pointer to an offset or does it
        //               interpret the integer value of the pointer as the actual offset
        //               ---
        //               Answer: https://wikis.khronos.org/opengl/Vertex_Rendering#Basic_Drawing
        //               The indices parameter is odd. Much like old-style vertex attributes, it is not a pointer at all.
        //               It is in fact a byte offset, which is disguised as a pointer. So you need to take your byte
        //               offset into the index buffer and cast it into a void* (with reinterpret_cast<void*> or
        //               just (void*)).
        glDrawElements(primitive_type, count, itype, reinterpret_cast<void*>(first_index));
    }

    void OpenglBackend::draw_elements_instanced(::graphics::PrimitiveType type, std::int32_t count,
                                                ::graphics::IndexType index_type, std::int32_t first_index,
                                                std::int32_t instances) {
        GLenum const primitive_type = map_primitive_type(type);
        GLenum const itype = map_index_type(index_type);
        glDrawElementsInstanced(primitive_type, count, itype, reinterpret_cast<void*>(first_index), instances);
    }

    constexpr bool parse_env_true_false(std::string const& env, const bool def) {
        auto evariable = std::getenv(env.c_str());
        if (evariable != nullptr) {
            std::string env_var = evariable;
            if (env_var == "true") {
                return true;
            }
            if (env_var == "false") {
                return false;
            }
        }
        return def;
    }

    void OpenglBackend::setup_from_env() {
        m_enable_validation = parse_env_true_false("ENGINE_BACKEND_OPENGL_VALIDATION", false);
    }
}
