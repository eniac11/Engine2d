#include "mainloop.h"

#include <vfspp/NativeFileSystem.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <engine/Camera.h>
#include <engine/2d/TileSet.h>
#include <engine/2d/tileset_mesher.h>
#include <engine/RAII/binding.h>
#include <engine/resources/dwarfs_fs.h>
#include <engine/elog.h>
#include <engine/2d/Chunk.h>

#include <iostream>
#include <array>
#include <string_view>
#include <memory>
#include <print>
#include <numeric>
#include <generator>


#include "sprite_render_shader2.h"
#include "tilemap_parse_temp.h"


ELOG_DECLARE_LOGGING_CATEGORY(lcGameApp, "game_app")

inline constexpr auto static_mesh = TileSetMesher<16, 16>::generate_mesh();


void set_window_hints() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

bool check_extensions() {
    // if ( !GLEW_ARB_bindless_texture ){
    //     // glGetTextureHandleARB()
    //     std::println("GLEW_ARB_bindless_texture NOT supported");
    //     return false;
    // }

    return true;
}


// glm::mat4 DrawSprite(glm::vec2 position,
//   glm::vec2 size, float rotate, glm::vec3 color) {
//     glm::mat4 model = glm::mat4(1.0f);
//     model = glm::translate(model, glm::vec3(position, 0.0f));
//
//     model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y,
//     0.0f)); model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f,
//     0.0f, 1.0f)); model = glm::translate(model, glm::vec3(-0.5f * size.x,
//     -0.5f * size.y, 0.0f));
//
//     model = glm::scale(model, glm::vec3(size, 1.0f));
//     return model;

// }

std::generator<std::uint32_t> generate_tileids(int bound) {
    for (auto const& i : std::views::iota(1, bound+1)) {
        co_yield i;
        co_yield i;
        co_yield i;
        co_yield i;
    }
}

void MyApp::setup() {
    // elog::Elogger::global_logger().install_log_handler(elog::handlers::sd_journal_handler);
    elog::Elogger::global_logger().install_filter_rules("engine.backends.*.bindings{debug}=false");
    // if (!m_virtualFileSystem->CreateFileSystem<dwarfs_fs>("/data", "data/assets.dwarfs")) {
    //     std::println(elogDebug(), "Could not create dwarfs filesystem");
    // }
    if (!m_virtualFileSystem->CreateFileSystem<vfspp::NativeFileSystem>("/data", "data")) {
        std::println(elogDebug(), "Could not create native filesystem");
    }


    camera = OrthographicCamera(WINDOW_WIDTH, WINDOW_HEIGHT);
    camera.set_far_plane(1.0f);
    camera.set_near_plane(-1.0f);
    camera.set_position(glm::vec3(0.0f, 0.0f, 1.0f));

    // program = m_graphics_backend->load_program_from_spirv(ShaderHandle::create<sprite_render_shader>());


    auto vertices_buffer = m_graphics_backend->create_buffer(2);
    vertices_buffer[0]->upload(static_mesh.first.size() * sizeof(Vertex2) + 1, static_mesh.first.data(),
                               GL_DYNAMIC_DRAW);
    vertices_buffer[0]->set_name(std::string_view("static_mesh_vertices"));
    vertices_buffer[1]->upload(static_mesh.second.size() * sizeof(std::uint32_t) + 1, static_mesh.second.data(),
                               GL_DYNAMIC_DRAW);
    vertices_buffer[1]->set_name(std::string_view("static_mesh_indicies"));
    tileid_buffer1 = m_graphics_backend->create_buffer();
    tileid_buffer2 = m_graphics_backend->create_buffer();
    std::vector<std::uint32_t> tile_ids;
    tile_ids.reserve(16*16*4);
    tile_ids.append_range(generate_tileids(16 * 16));
    // std::ranges::iota(tile_ids, 1);
    std::array<std::uint32_t, 16 * 16 * 4> tile_ids2{};
    tile_ids2.fill(22);

    tileid_buffer1->set_name(std::string_view("tileid_buffer1"));
    tileid_buffer2->set_name(std::string_view("tileid_buffer2"));
    // tileid_buffer->allocate(sizeof(std::uint32_t)*16*16*4+sizeof(std::uint32_t)*16*16*4, GL_DYNAMIC_DRAW);
    tileid_buffer1->upload(sizeof(std::uint32_t) * 16 * 16 * 4, tile_ids.data(), GL_DYNAMIC_DRAW);
    tileid_buffer2->upload(sizeof(std::uint32_t) * 16 * 16 * 4, tile_ids2.data(), GL_DYNAMIC_DRAW);

    vao2 = m_graphics_backend->create_vao("layer1_vao");
    vao2->set_ebo(vertices_buffer[1]);

    const auto vbuffer = vao2->add_vertex_data_buffer(vertices_buffer[0], sizeof(Vertex2));

    vao2->attrib_format(sprite_render_shader2::get_input_location("vertex"), GL_FLOAT, false);
    vao2->attrib_format(sprite_render_shader2::get_input_location("texCoord"), GL_FLOAT, false, offsetof(Vertex2, uv));

    if (auto hnd = vbuffer.lock()) {
        vao2->bind_buffer_to_attrib(hnd, sprite_render_shader2::get_input_location("vertex").location);
        vao2->bind_buffer_to_attrib(hnd, sprite_render_shader2::get_input_location("texCoord").location);
    }
    tileid_buffer1_handle = vao2->add_vertex_data_buffer(tileid_buffer1, sizeof(std::uint32_t));
    tileid_buffer2_handle = vao2->add_vertex_data_buffer(tileid_buffer2, sizeof(std::uint32_t));

    vao2->attrib_format(sprite_render_shader2::get_input_location("tileId"), GL_UNSIGNED_INT, false);
    // vao2->attrib_divisor(tilebuffer, 1);
    if (auto tb = tileid_buffer1_handle.lock()) {
        vao2->bind_buffer_to_attrib(tb, sprite_render_shader2::get_input_location("tileId").location);
    }


    // vfspp::IFilePtr file = m_virtualFileSystem->OpenFile("/data/sprite_render_shader2.spv", vfspp::IFile::FileMode::Read);
    //
    // size_t size = file->Size();
    // std::vector<uint8_t> shader_binary_data;
    // file->Read(shader_binary_data, size);
    // file->Close();
    program2 = m_graphics_backend->load_program_from_spirv(ShaderHandle::create<sprite_render_shader2>());

    program2->set_uniform(sprite_render_shader2::get_uniform_location("model"),
                          glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)), glm::vec3(32.0f, 32.0f, 1.0f)));
    program2->set_uniform(sprite_render_shader2::get_uniform_location("view"), camera.view());
    program2->set_uniform(sprite_render_shader2::get_uniform_location("projection"), camera.projection());

    vfspp::IFilePtr texture_file = m_virtualFileSystem->OpenFile("/data/final_set.ktx2", vfspp::IFile::FileMode::Read);
    texture2 = m_graphics_backend->create_texture_from_file(texture_file);
    texture_file->Close();
    texture2->parameter(TextureParameters::MAG_FILTER, GL_NEAREST);
    texture2->parameter(TextureParameters::MIN_FILTER, GL_NEAREST);

    auto f = m_virtualFileSystem->OpenFile("/data/tilemap", vfspp::IFile::FileMode::Read);
    std::vector<uint8_t> text(f->Size());
    f->Read(text);
    f->Close();
    std::string tilemapstring(text.begin(), text.end());

    tilemap_parse_temp temp_parser;
    std::vector<uint32_t> tilemap_tileids = temp_parser.parse(tilemapstring, {{'#', 25}, {'_', 22}});
    std::println(elogInfo(), "size: {}", tilemap_tileids.size());
    std::println(elogInfo(), "tilemap_tileids: {}", tilemap_tileids);
    layer1 = new TilemapLayer(m_graphics_backend.get(), 16, 16, 1);
    layer1->set_tileids(tilemap_tileids);
    layer1->finalise(m_graphics_backend.get());



    m_renderer = TilemapRenderer::create(m_graphics_backend.get());

    glEnable(GL_FRAMEBUFFER_SRGB);
    #ifdef DEBUG
    // Make it easier to find all the loading and setup gl calls in RenderDoc.
    glFinish();
    #endif
    glEnable(GL_DEPTH_TEST);
}

void MyApp::shutdown() {
    WindowedApp::shutdown();
    delete layer1;
    layer1 = nullptr;
}


void MyApp::update(float dt) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
        BindingHandle handle(texture2, 0);

        m_renderer->begin_drawing(1, camera);
        m_renderer->draw(m_graphics_backend.get(), *layer1);
        m_renderer->end_drawing();

        // BindingHandle vao_binding(vao2);
        // BindingHandle program_binding(program2);
        //
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("model"),
        //                       glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)),
        //                                  glm::vec3(32.0f, 32.0f, 1.0f)));
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("layer"), 1.0f);
        //
        // if (auto tb = tileid_buffer1_handle.lock()) {
        //     vao2->bind_buffer_to_attrib(tb, sprite_render_shader2::get_input_location("tileId").location);
        //     // glDrawElements(GL_TRIANGLES, 16 * 16 * 6, GL_UNSIGNED_INT, nullptr);
        //     m_graphics_backend->draw_elements(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0);
        //
        // }
        // // glVertexArrayAttribBinding(vao2->get_id(), sprite_render_shader2::get_input_location("tileId").location, tileid_binding_index1);
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("model"),
        //                       glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(32.0f * 16, 0.0f, 0.0f)),
        //                                  glm::vec3(32.0f, 32.0f, 1.0f)));
        // program2->set_uniform(sprite_render_shader2::get_uniform_location("layer"), 0.5f);
        // if (auto tb = tileid_buffer2_handle.lock()) {
        //     vao2->bind_buffer_to_attrib(tb, sprite_render_shader2::get_input_location("tileId").location);
        //     // m_graphics_backend->draw_elements(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0);
        //     glDrawElements(GL_TRIANGLES, 16 * 16 * 6, GL_UNSIGNED_INT, nullptr);
        // }
    }
}
