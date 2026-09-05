
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

thread_local std::shared_ptr<Buffer> vertices_buffer = nullptr;
thread_local std::shared_ptr<Buffer> indices_buffer  = nullptr;
thread_local bool initialised = false;



// Chunk::Chunk(std::weak_ptr<VertexArray::BufferHandle> handle, chunkid_t chunkid, std::array<std::uint32_t, 16*16> ids) : tile_ids(ids), tileid_buffer_handle(std::move(handle)), chunk_id(chunkid) {
// }

TilemapLayer::TilemapLayer(graphics::GraphicsBackend* backend, int const width, int const height, int const layer) : m_width(width), m_height(height), mn_layer(layer) {
    std::println(elogCInfo(lcEngine2d), "Setting Attribs");
    layer_vao = backend->create_vao();
    layer_vao->attrib_format(tilelayer_shader::get_input_location("vertex"), GL_FLOAT, false, offsetof(Vertex2, pos));
    layer_vao->attrib_format(tilelayer_shader::get_input_location("texCoord"), GL_FLOAT, false, offsetof(Vertex2, uv));
    // layer_vao->attrib_format(tilelayer_shader::get_input_location("tileId"), GL_UNSIGNED_INT, false);
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

void TilemapLayer::set_tileid_for_chunk(chunkid_t chunkid, std::array<std::uint32_t, 16*16*4> ids, std::span<std::array<uint32_t, 16 * 16 * 4>> stacks) {
    // assert(!stacks.empty());
    assert(!ids.empty());
    auto& chunk = chunks.emplace_back(ids, chunkid);


    if (!stacks.empty()) {
        chunk.n_tile_stacks = stacks.size();
        for (auto const& id : stacks) {
            chunk.extra_stacks.push_back(id);


        }
    }
}

void TilemapLayer::setup_shared_buffers(graphics::GraphicsBackend* backend) {
    if (!initialised) {
        std::println(elogCInfo(lcEngine2d), "Setting up shared buffers");
        vertices_buffer = backend->create_buffer();
        vertices_buffer->set_name(std::string("static_vertex_buffer"));
        vertices_buffer->upload(mesh.first.size()*sizeof(decltype(mesh.first)::value_type), mesh.first.data(), GL_STATIC_DRAW);
        indices_buffer = backend->create_buffer();
        indices_buffer->set_name(std::string("static_index_buffer"));
        std::println(elogCInfo(lcEngine2d), "Index Count: {} Index size: {}", mesh.second.size(), mesh.second.size()*sizeof(decltype(mesh.second)::value_type));
        std::println(elogCInfo(lcEngine2d), "TilesetMesh Index Count: {}", TileSetMesher<16,16>::indices_count);
        // glFinish();
        indices_buffer->upload(mesh.second.size()*sizeof(decltype(mesh.second)::value_type), mesh.second.data(), GL_STATIC_DRAW);
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
    constexpr std::size_t chunk_size = sizeof(decltype(Chunk::tile_ids));
    int i = 0;
    for (auto & chunk : chunks) {
        // auto buffer = backend->create_buffer();
        std::size_t n_extra = chunk.n_tile_stacks;
        // Allocate a buffer with size being 1+tile_stacks chunk size
        // buffer->allocate(chunk_size*(1+n_extra), GL_DYNAMIC_DRAW);
        // buffer->upload_subdata(chunk_size, 0, chunk.tile_ids.data());
        chunk.tile_stack_ssbo = std::make_unique<SSBO<decltype(Chunk::tile_ids)>>(0, backend, 1+n_extra);
        chunk.tile_stack_ssbo->update(0, std::span(chunk.tile_ids.begin(), chunk.tile_ids.end()));

        // auto handle = layer_vao->add_vertex_data_buffer(buffer, sizeof(uint32_t));
        // chunk.tileid_buffer_handle = handle;

        // if (auto h = handle.lock()) {
        //     layer_vao->attrib_divisor(h, 1);
        // }
        // if (!chunk.extra_tileid_stacks.empty()) {
         size_t n = 1;
        for (auto const& id : chunk.extra_stacks) {
            // auto b = backend->create_buffer();
            // b->allocate(sizeof(decltype(Chunk::tile_ids)), GL_STATIC_DRAW);
            chunk.tile_stack_ssbo->update(chunk_size*n, std::span(id.begin(), id.end()));
            // auto h = layer_vao->add_vertex_data_buffer(b, sizeof(uint32_t));
            // chunk.extra_tileid_stacks.push_back(h);
            n++;
        }
        // }
    }
    // for (auto const& tile_chunk : tile_ids | std::views::chunk(16*16*4)) {
    //     std::println(elogCInfo(lcEngine2d), "size: {}", tile_chunk.size());
    //     std::println(elogCInfo(lcEngine2d), "tile_chunk: {}", tile_chunk);
    //     Transform2d _transform;
    //     _transform.position({static_cast<float>(i)*32.0f*16, 0.0f});
    //     elogCInfo(lcEngine2d) << _transform.position() << std::endl;
    //     _transform.scale({32.0f, 32.0f});
    //     auto buffer = backend->create_buffer();
    //     buffer->allocate(sizeof(uint32_t)*16*16*4, GL_DYNAMIC_DRAW);
    //     buffer->upload_subdata(sizeof(uint32_t)*tile_chunk.size(), 0, tile_chunk.data());
    //     auto handle = layer_vao->add_vertex_data_buffer(buffer, sizeof(uint32_t));
    //
    //     chunks.emplace_back(handle, _transform, tile_chunk);
    //     i++;
    // }
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
    Transform2d tr = layer.transform;
    tr.scale({32.0f, 32.0f});
    m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("model"), tr.transformation_matrix());

    if (layer.chunks.empty())
        elogCWarningEnabled(lcEngine2d) std::println(elogCWarning(lcEngine2d), "TilemapLayer({}) does not have any chunks generated.", layer.mn_layer);
    // std::println(elogCInfo(lcEngine2d), "Drawing TilemapLayer({}): chunks {}", layer.mn_layer, layer.chunks.size());
    for (auto& chunk : layer.chunks) {
        // m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("model"), chunk.transform.transformation_matrix());
        m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("chunk_id"), chunk.chunk_id);
        m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("stack_count"), chunk.n_tile_stacks);
        // glShaderStorageBlockBinding(m_tilelayer_shader->get_id(), 0, 1);
        // m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("stack"), 0u);

        // BindingHandle ssbo(chunk.tile_stack_buffer);
        // SSBO<uint32_t> ssbo(0, chunk.tile_stack_buffer);
        chunk.tile_stack_ssbo->bind();
        // chunk.tile_stack_buffer.
         // if (auto buffer_handle = chunk.tileid_buffer_handle.lock()) {
            // layer.layer_vao->bind_buffer_to_attrib(buffer_handle, tilelayer_shader::get_input_location("tileId").location);
            // TODO: use glDrawElementsBaseVertex allows vertex offset to be independent of index or
            //       allows the same index to reference different vertex data
            // TODO: use glMultiDrawElementsBaseVertex
            // FIXME: remove magic number
        backend->draw_elements_instanced(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0, 1+std::int32_t(chunk.n_tile_stacks));
        // chunk.tile_stack_ssbo->unbind();
        // }
        // uint32_t stack_count = 0;
        // if (!chunk.extra_tileid_stacks.empty()) {
        //     stack_count = chunk.extra_tileid_stacks.size();
        // }
        // m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("stack_count"), stack_count);
        // // std::println(elogCInfo(lcEngine2d), "Drawing TilemapLayer({}): extra stacks {}", layer.mn_layer, chunk.extra_tileid_stacks.size());
        //
        // uint32_t stack = chunk.extra_tileid_stacks.size();
        // for (auto& wh : chunk.extra_tileid_stacks) {
        //     if (auto h = wh.lock()) {
        //         layer.layer_vao->bind_buffer_to_attrib(h, tilelayer_shader::get_input_location("tileId").location);
        //     }
        //     m_tilelayer_shader->set_uniform(tilelayer_shader::get_uniform_location("stack"), stack);
        //
        //     backend->draw_elements(graphics::PrimitiveType::TRIANGLES, 16*16*6, graphics::IndexType::UNSIGNED_INT, 0);
        //     stack--;
        // }

    }

}

void TilemapRenderer::end_drawing() {
    m_bound_handle.reset();
}
