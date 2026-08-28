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


class TilemapRenderer;
class TilemapLayer;

class Chunk {
    friend class TilemapRenderer;
    // FIXME: This is temporary for debugging purposes
    friend class TilemapLayer;
    Transform2d transform;
    std::span<std::uint32_t> tile_ids;
    std::weak_ptr<VertexArray::BufferHandle> tileid_buffer_handle;

    public:
        Chunk(std::weak_ptr<VertexArray::BufferHandle> handle, Transform2d transform, std::span<std::uint32_t> ids);
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

    std::shared_ptr<Buffer> vertices_buffer;
    std::shared_ptr<Buffer> indices_buffer;
    bool initialised = false;

    public:
        TilemapLayer(graphics::GraphicsBackend* backend, int width, int height, int layer);
        void set_tileids(std::span<uint32_t> ids);
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