#include "engine/models/Mesh.h"

#include <glm/gtx/io.hpp>
// #include <glm/gtc/type_aligned.hpp>
#include <iostream>

Mesh::Mesh(int mesh_name, VectorOfVertices vertices, std::vector<unsigned> indices, std::vector<std::shared_ptr<Texture>> textures, glm::mat4 transform) : m_vertices(std::move(vertices)),
                                                                       m_indices(std::move(indices)), m_vao(VertexArray::create()), m_transform(transform), m_textures(std::move(textures)), m_mesh_name(mesh_name) {
    setupMesh();
}

Mesh::Mesh(int mesh_name, VectorOfVertices vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures, const std::string& name, glm::mat4 transform) : Mesh(mesh_name, std::move(vertices), std::move(indices), textures, transform) {
    glObjectLabel(GL_VERTEX_ARRAY, m_vao->get_id(), -1, name.c_str());
}


void Mesh::setupMesh() {
    auto buffers = Buffer::create(2);
    m_ebo = buffers[0];
    m_ebo->upload(m_indices.size()*sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
    m_vao->set_ebo(m_ebo);
    m_vbo = buffers[1];
    m_vbo->allocate(m_vertices.buffer_stride(), GL_STATIC_DRAW);
    // m_vbo->upload(m_vertices.positions.size(), m_vertices.positions.data(), GL_NONE);
    m_vbo->upload_subdata(m_vertices.positions_size(), 0, m_vertices.positions.data());
    m_vbo->upload_subdata(m_vertices.normals_size(), m_vertices.positions_size(), m_vertices.normals.data());
    m_vbo->upload_subdata(m_vertices.texCoords_size(), m_vertices.positions_size() + m_vertices.normals_size(), m_vertices.texCoords.data());

    auto const& position_binding = m_vao->add_vertex_buffer(m_vbo, sizeof(glm::vec3));
    auto const& normals_binding = m_vao->add_vertex_buffer(m_vbo, sizeof(glm::vec3), m_vertices.positions_size());
    auto const& texcoords_binding = m_vao->add_vertex_buffer(m_vbo, sizeof(glm::vec2), m_vertices.positions_size() + m_vertices.normals_size());

    m_vao->attrib(position_binding, 3, 0, GL_FLOAT, false, 0);
    m_vao->attrib(normals_binding, 3, 1, GL_FLOAT, false, 0);
    m_vao->attrib(texcoords_binding, 2, 2, GL_FLOAT, false, 0);
}

int Mesh::mesh_name() const {
    return m_mesh_name;
}

void Mesh::draw() {
    // program.set_uniform(model_uniform, m_transform);
    auto vao_binding = VertexArrayBindingHandle(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    // m_vao.unbind();
}
