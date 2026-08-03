#include "engine/2d/TileSet.h"

#include <LDtkLoader/Project.hpp>

TileSet::TileSet(std::shared_ptr<Texture> texture, glm::ivec2 tile_size, glm::ivec2 margin) :
    m_texture(std::move(texture)), m_tile_size(tile_size), m_margin(margin) {
}

TileSet::TileUv TileSet::get_uv(glm::uvec2 pos) const {


    // FIXME(hadley): Account for margin
    // Reflect the pos parameter type in case, type of pos is changed
    using itype = decltype(pos)::value_type;

    const itype flattened_coord = pos.x * m_tile_size.y;

    return TileUv{flattened_coord, glm::mat4(1.0f)};

}

std::size_t TileSet::size() const {
    // FIXME(hadley): Account for margin
    const int tiles_x_count = m_texture->width() / m_tile_size.x;
    const int tiles_y_count = m_texture->height() / m_tile_size.y;
    return tiles_x_count * tiles_y_count;
}

std::shared_ptr<Texture> TileSet::texture() {
    return m_texture;
}
