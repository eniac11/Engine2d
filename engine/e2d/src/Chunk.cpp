
#include <engine/2d/Chunk.h>
#include <glm/gtx/io.hpp>
#include <engine/2d/tileset_mesher.h>
#include <engine/RAII/binding.h>

#include "tilelayer_shader.h"
#include "e2d_logging.h"
#include <iostream>

#include "engine/Camera.h"


// FIXME: Remove magic number
static auto mesh = TileSetMesher<16, 16>::generate_mesh();


Chunk::Chunk(std::weak_ptr<VertexArray::BufferHandle> handle, Transform2d transform, std::span<std::uint32_t> ids) : transform(transform), tile_ids(ids), tileid_buffer_handle(std::move(handle)) {
}

TilemapLayer::TilemapLayer(graphics::GraphicsBackend* backend, int const width, int const height, int const layer) : m_width(width), m_height(height), mn_layer(layer) {
    std::println(elogCInfo(lcEngine2d), "Setting Attribs");
    layer_vao = backend->create_vao();
    layer_vao->attrib_format(tilelayer_shader::get_input_location("vertex"), GL_FLOAT, false, offsetof(Vertex2, pos));
    layer_vao->attrib_format(tilelayer_shader::get_input_location("texCoord"), GL_FLOAT, false, offsetof(Vertex2, uv));
    layer_vao->attrib_format(tilelayer_shader::get_input_location("tileId"), GL_UNSIGNED_INT, false);
    // NOTE: This is thread local
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

void TilemapLayer::set_tileids(std::span<uint32_t> ids) {
    tile_ids.clear();
    std::ranges::copy(ids, std::back_inserter(tile_ids));
}

void TilemapLayer::setup_shared_buffers(graphics::GraphicsBackend* backend) {
    if (!initialised) {
        std::println(elogCInfo(lcEngine2d), "Setting up shared buffers");
        vertices_buffer = backend->create_buffer();
        vertices_buffer->set_name(std::string("static_vertex_buffer"));
        vertices_buffer->upload(mesh.first.size()*sizeof(decltype(mesh.first)::value_type), mesh.first.data(), GL_DYNAMIC_DRAW);
        indices_buffer = backend->create_buffer();
        indices_buffer->set_name(std::string("static_index_buffer"));
        std::println(elogCInfo(lcEngine2d), "Index Count: {} Index size: {}", mesh.second.size(), mesh.second.size()*sizeof(decltype(mesh.second)::value_type));
        std::println(elogCInfo(lcEngine2d), "TilesetMesh Index Count: {}", TileSetMesher<16,16>::indices_count);
        // glFinish();
        indices_buffer->upload(mesh.second.size()*sizeof(decltype(mesh.second)::value_type), mesh.second.data(), GL_DYNAMIC_DRAW);
        // glFinish();
        initialised = true;
    }
}

bool safe_advance(std::random_access_iterator auto& it, std::random_access_iterator auto& end, std::size_t n) {
    auto remaining = std::distance(it, end);
    if (remaining < n) {
        n = remaining;
    }
    std::advance(it, n);
    if (it != end) {
        return true;
    }
    return false;
}

void TilemapLayer::finalise(graphics::GraphicsBackend* backend) {
    int i = 0;
    for (auto const& tile_chunk : tile_ids | std::views::chunk(16*16*4)) {
        std::println(elogCInfo(lcEngine2d), "size: {}", tile_chunk.size());
        std::println(elogCInfo(lcEngine2d), "tile_chunk: {}", tile_chunk);
        Transform2d _transform;
        _transform.position({static_cast<float>(i)*32.0f*16, 0.0f});
        elogCInfo(lcEngine2d) << _transform.position() << std::endl;
        _transform.scale({32.0f, 32.0f});
        auto buffer = backend->create_buffer();
        buffer->allocate(sizeof(uint32_t)*16*16*4, GL_DYNAMIC_DRAW);
        buffer->upload_subdata(sizeof(uint32_t)*tile_chunk.size(), 0, tile_chunk.data());
        auto handle = layer_vao->add_vertex_data_buffer(buffer, sizeof(uint32_t));

        chunks.emplace_back(handle, _transform, tile_chunk);
        i++;
    }
    // elogCInfo(lcEngine2d) << chunks.at(0).transform.transformation_matrix() << std::endl;
    // elogCInfo(lcEngine2d) << chunks.at(0).transform.position() << std::endl;
    // elogCInfo(lcEngine2d) << chunks.at(0).transform.scale() << std::endl;
    // void* mapping = glMapNamedBuffer(indices_buffer->get_id(),  GL_READ_ONLY);
    // auto* data = static_cast<std::uint32_t*>(mapping);
    //
    // std::span<std::uint32_t> tile_ids(data, mesh.second.size());
    // std::println(elogCInfo(lcEngine2d), "CPU Memory: {}", mesh.second);
    // std::println(elogCInfo(lcEngine2d), "GPU Memory: {}", tile_ids);
    //
    //
    //
    //
    //
    // glUnmapNamedBuffer(indices_buffer->get_id());
    //     auto* data = static_cast<std::uint32_t*>(mapping);
    //     std::span<std::uint32_t> tile_ids(data, chunks.at(0).tile_ids.size());
    //     std::println(elogCInfo(lcEngine2d), "CPU Memory: {}", chunks.at(0).tile_ids);
    //     std::println(elogCInfo(lcEngine2d), "GPU Memory: {}", tile_ids);
    //     glUnmapNamedBuffer(h->buffer->get_id());
    // if (auto h = chunks.at(0).tileid_buffer_handle.lock())
    // {
    //     void* mapping = glMapNamedBuffer(h->buffer->get_id(),  GL_READ_ONLY);
    //     auto* data = static_cast<std::uint32_t*>(mapping);
    //     std::span<std::uint32_t> tile_ids(data, chunks.at(0).tile_ids.size());
    //     std::println(elogCInfo(lcEngine2d), "CPU Memory: {}", chunks.at(0).tile_ids);
    //     std::println(elogCInfo(lcEngine2d), "GPU Memory: {}", tile_ids);
    //     glUnmapNamedBuffer(h->buffer->get_id());
    //
    // }

}

TilemapRenderer::TilemapRenderer(std::shared_ptr<ShaderProgram> shader) : m_tilelayer_shader(std::move(shader)) {
}

std::shared_ptr<TilemapRenderer> TilemapRenderer::create(graphics::GraphicsBackend* backend) {
    auto program = backend->load_program_from_spirv(ShaderHandle::create<tilelayer_shader>());

    return std::make_shared<TilemapRenderer>(program);
}

void TilemapRenderer::begin_drawing(uint32_t layer_count, Camera const& camera) {
    // m_bound_handle.reset(new BindingHandle(m_tilelayer_shader));
    m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("layer_count"), layer_count);
    m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("projection"), camera.projection());
    m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("view"), camera.view());
}

void TilemapRenderer::draw(graphics::GraphicsBackend* backend, TilemapLayer const& layer) {
    // Always bind shader because the current shader may have changed as layers are separate from the renderer and
    // other rendering code may have run.
    BindingHandle shader_handle(m_tilelayer_shader);
    BindingHandle handle(layer.layer_vao);
    m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("layer"), layer.mn_layer);
    if (layer.chunks.empty())
        elogCWarningEnabled(lcEngine2d) std::println(elogCWarning(lcEngine2d), "TilemapLayer({}) does not have any chunks generated.", layer.mn_layer);
    for (auto const& chunk : layer.chunks) {
        m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("model"), chunk.transform.transformation_matrix());
        if (auto buffer_handle = chunk.tileid_buffer_handle.lock()) {
            layer.layer_vao->bind_buffer_to_attrib(buffer_handle, tilelayer_shader::get_input_location("tileId").location);
        }
        // TODO: use glDrawElementsBaseVertex allows vertex offset to be independent of index or
        //       allows the same index to reference different vertex data
        // TODO: use glMultiDrawElementsBaseVertex
        // FIXME: remove magic number
        backend->draw_elements(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0);
    }

}

void TilemapRenderer::end_drawing() {
    m_bound_handle.reset();
}
