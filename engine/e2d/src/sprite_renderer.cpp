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

#include "engine/2d/sprite_renderer.h"

#include <engine/RAII/binding.h>

#include "sprite_shader.h"

struct Vertex {
    glm::vec2 pos;
    glm::vec2 uv;
};

constexpr Vertex vertices[] = {
    {{1.0f, 1.0f}, {1.0f, 1.0f}},
    {{1.0f, -1.0f}, {1.0f, 0.0f}},
    {{-1.0f, -1.0f}, {0.0f, 0.0f}},
    {{-1.0f, 1.0f}, {0.0f, 1.0f}}


};

constexpr uint32_t index_data[] = {
    0, 1, 3,
    1, 2, 3
};

sprite_renderer::sprite_renderer(graphics::GraphicsBackend& backend)
    : m_sprite_shader(backend.load_program_from_spirv(ShaderHandle::create<sprite_shader>())),
      m_sprite_ubo(0, &backend) {
    auto vertex_buffer = backend.create_buffer();
    vertex_buffer->upload(sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    auto index_buffer = backend.create_buffer();
    index_buffer->upload(sizeof(index_data), index_data, GL_DYNAMIC_DRAW);
    m_vao = backend.create_vao();
    m_vao->set_ebo(index_buffer);
    auto whandle = m_vao->add_vertex_data_buffer(vertex_buffer, sizeof(Vertex));
        m_vao->attrib_format(sprite_shader::get_input_location("vertex"), GL_FLOAT, false, offsetof(Vertex, pos));
        m_vao->attrib_format(sprite_shader::get_input_location("texCoord"), GL_FLOAT, false,offsetof(Vertex, uv));
    if (auto handle = whandle.lock()) {
        m_vao->bind_buffer_to_attrib(handle, sprite_shader::get_input_location("vertex").location);
        m_vao->bind_buffer_to_attrib(handle, sprite_shader::get_input_location("texCoord").location);
    }

}

void sprite_renderer::update_matrices(Camera const& camera) {
    m_sprite_shader->set_uniform(sprite_shader::get_uniform_location("projection"), camera.projection());
    m_sprite_shader->set_uniform(sprite_shader::get_uniform_location("view"), camera.view());
}

void sprite_renderer::render(graphics::GraphicsBackend& backend, Sprite const& sprite, Transform2d const& transform) {
    GLboolean depth_test = GL_FALSE;
    glGetBooleanv(GL_DEPTH_TEST, &depth_test);
    if (depth_test == GL_TRUE) {
        glDisable(GL_DEPTH_TEST);
    }

    m_sprite_ubo.update(GPUSprite(sprite, transform));
    m_sprite_ubo.bind();
    BindingHandle shader_handle(m_sprite_shader);
    BindingHandle vao_handle(m_vao);
    backend.draw_elements(graphics::PrimitiveType::TRIANGLES, 6, graphics::IndexType::UNSIGNED_INT, 0);
    m_sprite_ubo.unbind();
    if (depth_test == GL_TRUE) {
        glEnable(GL_DEPTH_TEST);
    }
}
