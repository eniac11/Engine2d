#pragma once

#include <engine/graphics/VertexArray.h>
#include <engine/math.h>
#include <engine/graphics/backend.h>
#include <engine/RAII/binding.h>


#include <glm/vec2.hpp>

#include <vector>
#include <string>
#include <memory>

#include "engine/Camera.h"
#include "engine/Camera.h"
#include "engine/Camera.h"
#include "engine/graphics/ubo.h"


class TilemapRenderer;
class TilemapLayer;

using chunkid_t = glm::ivec2;

struct Chunk {
    // friend class TilemapRenderer;
    // FIXME: This is temporary for debugging purposes
    // friend class TilemapLayer;
    std::array<std::uint32_t, 16*16*4> tile_ids;
    chunkid_t chunk_id;
    std::uint32_t n_tile_stacks = 0;
    std::vector<std::array<std::uint32_t, 16*16*4>> extra_stacks;
    // std::shared_ptr<Buffer> tile_stack_buffer;
    std::unique_ptr<SSBO<decltype(tile_ids)>> tile_stack_ssbo;
    // std::weak_ptr<VertexArray::BufferHandle> tileid_buffer_handle;
    // std::vector<std::weak_ptr<VertexArray::BufferHandle>> extra_tileid_stacks;

    // public:
        // Chunk(std::weak_ptr<VertexArray::BufferHandle> handle, chunkid_t chunkid, std::array<std::uint32_t, 16*16> ids);
};

class TilemapLayer {
    friend class TilemapRenderer;
    std::string layer_name;
    Transform2d transform;
    std::vector<Chunk> chunks;
    std::shared_ptr<VertexArray> layer_vao;
    std::vector<std::uint32_t> tile_ids;
    int m_width;
    int m_height;
    uint32_t mn_layer;



    public:
        TilemapLayer(graphics::GraphicsBackend* backend, int width, int height, int layer);
        void set_tileids(std::span<uint32_t> ids);
        void set_tileid_for_chunk(chunkid_t chunkid, std::array<std::uint32_t, 16*16*4>, std::span<std::array<unsigned, 1024>> ids);
        void setup_shared_buffers(graphics::GraphicsBackend* backend);
        void finalise(graphics::GraphicsBackend* backend);
};

class TilemapRenderer {
    std::shared_ptr<ShaderProgram> m_tilelayer_shader;
    std::shared_ptr<BindingHandle<ShaderProgram>> m_bound_handle;
    public:
    explicit TilemapRenderer(std::shared_ptr<ShaderProgram> shader);

    static std::shared_ptr<TilemapRenderer> create(graphics::GraphicsBackend* backend);
    void begin_drawing(uint32_t layer_count, Camera const& camera);
    void draw(graphics::GraphicsBackend* backend, TilemapLayer const& layer);
    void end_drawing();
};