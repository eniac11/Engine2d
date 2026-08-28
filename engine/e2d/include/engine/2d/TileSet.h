#pragma once
#include <memory>

#include <engine/graphics/Texture.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class TileSet {
    public:
        struct TileUv {
            std::uint32_t tileId;
            glm::mat4 model;
        };

        TileSet(std::shared_ptr<Texture> texture, glm::ivec2 tile_size, glm::ivec2 margin = {0, 0});

        /**
         * The number of tiles in the tilemap
         * @return number of tiles
         */
        std::size_t size() const;
        std::shared_ptr<Texture> texture();

    private:
        std::shared_ptr<Texture> m_texture;
        glm::ivec2 m_tile_size;
        glm::ivec2 m_margin;
};
