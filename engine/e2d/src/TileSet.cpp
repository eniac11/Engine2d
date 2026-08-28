#include "engine/2d/TileSet.h"

// #include <LDtkLoader/Tileset.hpp>

TileSet::TileSet(std::shared_ptr<Texture> texture, glm::ivec2 const tile_size, glm::ivec2 const margin) :
    m_texture(std::move(texture)), m_tile_size(tile_size), m_margin(margin) {
}

std::size_t TileSet::size() const {
    if (m_texture->has_layers()) {
        return m_texture->layers();
    }
    const int tiles_x_count = m_texture->width() / m_tile_size.x;
    const int tiles_y_count = m_texture->height() / m_tile_size.y;
    return tiles_x_count * tiles_y_count;
}

std::shared_ptr<Texture> TileSet::texture() {
    return m_texture;
}
