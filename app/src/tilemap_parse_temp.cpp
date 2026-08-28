// 2d zero-code game engine
// Copyright (C) 2026. Hadley Epstein
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "tilemap_parse_temp.h"

std::vector<std::uint32_t> tilemap_parse_temp::parse(std::string const& tilemaptemplate,
    std::map<char, std::uint32_t> rep_to_tile_id) {
    std::vector<std::uint32_t> tiles;
    for (auto const & tile : tilemaptemplate) {
        if (tile == '\n')
            continue;
        tiles.push_back(rep_to_tile_id[tile]);
        tiles.push_back(rep_to_tile_id[tile]);
        tiles.push_back(rep_to_tile_id[tile]);
        tiles.push_back(rep_to_tile_id[tile]);
    }
    return tiles;
}
