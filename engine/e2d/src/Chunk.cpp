
#include <engine/2d/Chunk.h>

#include <engine/2d/tileset_mesher.h>
#include "tilelayer_shader.h"



static auto mesh = TileSetMesher<16, 16>::generate_mesh();

thread_local std::shared_ptr<Buffer> vertices_buffer;
thread_local std::shared_ptr<Buffer> indices_buffer;
thread_local bool initialised = false;

TilemapLayer::TilemapLayer(graphics::GraphicsBackend* backend, int const width, int const height) : m_width(width), m_height(height) {
    layer_vao = backend->create_vao();
    layer_vao->attrib_format(tilelayer_shader::get_input_location("vertex"), GL_FLOAT, false, offsetof(Vertex2, pos));
    layer_vao->attrib_format(tilelayer_shader::get_input_location("texCoord"), GL_FLOAT, false, offsetof(Vertex2, uv));
    layer_vao->attrib_format(tilelayer_shader::get_input_location("tileId"), GL_UNSIGNED_INT, false);
    if (!initialised) {
        setup_shared_buffers(backend);
    }
    {
        auto whandle = layer_vao->add_vertex_data_buffer(vertices_buffer, sizeof(decltype(mesh.first)::value_type));
        if (auto handle = whandle.lock()) {
            layer_vao->bind_buffer_to_attrib(handle, tilelayer_shader::get_input_location("vertex").location);
            layer_vao->bind_buffer_to_attrib(handle, tilelayer_shader::get_input_location("texCoord").location);
        }
    }
    layer_vao->set_ebo(indices_buffer);

}

void TilemapLayer::setup_shared_buffers(graphics::GraphicsBackend* backend) {
    if (!initialised) {
        vertices_buffer = backend->create_buffer();
        vertices_buffer->upload(mesh.first.size()*sizeof(decltype(mesh.first)::value_type), mesh.first.data(), GL_DYNAMIC_DRAW);
        indices_buffer = backend->create_buffer();
        vertices_buffer->upload(mesh.second.size()*sizeof(decltype(mesh.second)::value_type), mesh.second.data(), GL_DYNAMIC_DRAW);

        initialised = true;
    }
}

void TilemapLayer::finalise() {

}
