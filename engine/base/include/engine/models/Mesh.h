#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "engine/ShaderProgram.h"
#include "engine/ShaderResources.h"
#include "engine/Texture.h"
#include "engine/graphics/Buffer.h"
#include "engine/graphics/VertexArray.h"
#include "engine/rendering/Material.h"

// #include "engine/Texture.h"


#define VECTOR_SIZE_WITH_STRIDE(element, T) (element.size() * sizeof(T))

struct VectorOfVertices {
    std::vector<glm::vec3> positions;
    size_t positions_size() const {
        return VECTOR_SIZE_WITH_STRIDE(positions, glm::vec3);
    }
    std::vector<glm::vec3> normals;
    size_t normals_size() const {
        return VECTOR_SIZE_WITH_STRIDE(normals, glm::vec3);
    }
    std::vector<glm::vec2> texCoords;
    size_t texCoords_size() const {
        return VECTOR_SIZE_WITH_STRIDE(texCoords, glm::vec2);
    }


    size_t buffer_stride() const {
        return positions_size() + normals_size() + texCoords_size();
    }
};


class Mesh {
    VectorOfVertices m_vertices;
    std::vector<unsigned int> m_indices;
    // std::vector<Texture> positions;
    std::shared_ptr<Buffer> m_vbo;
    std::shared_ptr<Buffer> m_ebo;
    std::shared_ptr<VertexArray> m_vao;
    glm::mat4 m_transform;
    std::vector<std::shared_ptr<Texture>> m_textures;
    int m_mesh_name;

    void setupMesh();

    public:
        Mesh(int mesh_name, VectorOfVertices vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures, glm::mat4 transform = glm::mat4());
        Mesh(int mesh_name, VectorOfVertices vertices, std::vector<unsigned int> indices, std::vector<std::shared_ptr<Texture>> textures, const std::string& name, glm::mat4 transform = glm::mat4());
        int mesh_name() const;
        glm::mat4 getTransform() const { return m_transform;}
        // Material const& get_material() const { return m_material;}
        void draw();
};
