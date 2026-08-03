
#pragma once

#include <engine/graphics/VertexArray.h>
#include <engine/math.h>
#include <engine/graphics/backend.h>


#include <glm/vec2.hpp>

#include <vector>
#include <string>
#include <memory>


class Chunk {
    Transform2d transform;
    std::span<std::uint32_t> tile_ids;
    std::weak_ptr<VertexArray::BufferHandle> tileid_buffer_handle;

    public:
        Chunk(graphics::GraphicsBackend* backend, Transform2d transform, std::span<std::uint32_t>);
};

class TilemapLayer {
    std::string layer_name;
    Transform2d transform;
    std::vector<Chunk> chunks;
    std::shared_ptr<VertexArray> layer_vao;
    std::vector<std::uint32_t> tile_ids;
    int m_width;
    int m_height;

    public:
        TilemapLayer(graphics::GraphicsBackend* backend, int width, int height);
        static void setup_shared_buffers(graphics::GraphicsBackend* backend);
        void finalise();
};
