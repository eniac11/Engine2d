#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <glm/vec2.hpp>
#include <utility>


struct [[gnu::packed]] Vertex2 {
    alignas(16) glm::vec2 pos;
    glm::vec2 uv;
};

template <int tWidth, int tHeight>
struct TileSetMesher {
    static constexpr int vertices_per_grid = 4;
    static constexpr int indices_per_grid = 6;
    static constexpr int height = tHeight;
    static constexpr int width = tWidth;
    static constexpr size_t tilecount = width * height;
    static constexpr size_t vertices_count = tilecount * vertices_per_grid+1;
    static constexpr size_t indices_count = tilecount * indices_per_grid+1;
    using vectype = glm::vec2;
    using indextype = std::uint32_t;
    using vertexarr_type = std::array<Vertex2, vertices_count>;
    using indexarr_type = std::array<indextype, indices_count>;

    static constexpr std::pair<vertexarr_type, indexarr_type> generate_mesh() {
        vertexarr_type vertices{};
        indexarr_type indices{};
        size_t i = 0;
        int j = 0;

        for (int y = 0; y < tHeight; y++) {
            for (int x = 0; x < tWidth; x++) {
                // bottom left
                vertices[i + 0] = {glm::vec2(x, y), glm::vec2{0.0f, 0.0f}};
                // bottom right
                vertices[i + 1] = {glm::vec2(x + 1, y), {1.0f, 0.0f}};
                // top left
                vertices[i + 2] = {glm::vec2(x, y + 1), {0.0f, 1.0f}};
                // top right
                vertices[i + 3] = {glm::vec2(x + 1, y + 1), {1.0f, 1.0f}};
                i += vertices_per_grid;
            }
        }
        i = 0;
        for (int y = 0; y < tHeight; y++) {
            for (int x = 0; x < tWidth; x++) {
                indices[i + 0] = j;
                indices[i + 1] = j + 1;
                indices[i + 2] = j + 2;

                indices[i + 3] = j + 1;
                indices[i + 4] = j + 2;
                indices[i + 5] = j + 3;
                i += indices_per_grid;
                j += vertices_per_grid;
            }
        }

        return std::make_pair(vertices, indices);
    }
};
