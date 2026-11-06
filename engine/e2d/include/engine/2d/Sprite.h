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

#pragma once
#include <engine/math.h>
#include <memory>
#include <engine/graphics/Texture.h>


struct Sprite {
    glm::vec4 colour;
    uint32_t tileid;
    std::shared_ptr<Texture> tileset;
};

struct GPUSprite {
    glm::mat4 transform;
    glm::vec4 colour;
    uint32_t tileid;

    explicit GPUSprite(Sprite const& sprite, Transform2d const& transform)
        : transform(transform.transformation_matrix()),
          colour(sprite.colour),
          tileid(sprite.tileid) {
    }
};
