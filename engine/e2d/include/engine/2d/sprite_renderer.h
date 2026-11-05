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

#include <engine/graphics/backend.h>
#include <engine/ShaderProgram.h>
#include <memory>
#include <engine/Camera.h>
#include <engine/graphics/ubo.h>
#include <engine/2d/Sprite.h>

class sprite_renderer {
    std::shared_ptr<ShaderProgram> m_sprite_shader;
    std::shared_ptr<VertexArray> m_vao;
    UBO<GPUSprite> m_sprite_ubo;
    public:
        explicit sprite_renderer(graphics::GraphicsBackend& backend);

        void update_matrices(Camera const& camera);

        void render(graphics::GraphicsBackend& backend, Sprite const& sprite);
};
