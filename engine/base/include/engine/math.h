
#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>

#include <type_traits>

template <typename posT, typename scaleT, typename rotT>
class Transform {
    protected:
    using pos_type = posT;
    using scale_type = scaleT;
    using rotation_type = rotT;

    pos_type m_position{};
    scale_type m_scale{};
    rotation_type m_rotation{};

    glm::mat4 m_transformation_matrix{1.0f};


    constexpr void update_matrix() {
        auto mat = glm::mat4(1.0f);

        if constexpr (std::is_same_v<pos_type, glm::vec2> && std::is_same_v<scale_type, glm::vec2> && std::is_same_v<rotation_type, float>) {
            auto translate = glm::translate(glm::mat4(1.0f), glm::vec3(m_position, 0.0f));
            auto rotate = glm::rotate(glm::mat4(1.0f), m_rotation, glm::vec3(0.0f, 0.0f, 1.0f));
            auto scale = glm::scale(glm::mat4(1.0f), glm::vec3(m_scale, 1.0f));
            mat = translate * rotate * scale;
        } else if constexpr (std::is_same_v<pos_type, glm::vec3> && std::is_same_v<scale_type, glm::vec3> && std::is_same_v<rotation_type, glm::quat>) {
            auto translate = glm::translate(mat, m_position);
            auto rotate = glm::mat4_cast(m_rotation);
            auto scale = glm::scale(mat, glm::vec3(m_scale, 1.0f));
            mat = translate * rotate * scale;
        }
        m_transformation_matrix = mat;

    }

    public:

        glm::mat4 transformation_matrix() const {
            return m_transformation_matrix;
        }

        pos_type local_to_world(pos_type pos) {
            if constexpr (std::is_same_v<pos_type, glm::vec2>) {
                auto const v4 = m_transformation_matrix * glm::vec4(glm::vec3(pos, 0.0f), 1.0f);
                return glm::vec2(v4);
            }
            return {};
        }
        pos_type world_to_local(pos_type pos) {
            if constexpr (std::is_same_v<pos_type, glm::vec2>) {
                auto const v4 = (glm::inverse(m_transformation_matrix) * glm::vec4(glm::vec3(pos, 0.0f), 0.0f));
                return glm::vec2(v4);
            }
            return {};
        }
    [[nodiscard]] pos_type position() const {
        return m_position;
    }

    void position(pos_type const& position) {
        this->m_position = position;
        update_matrix();
    }

    [[nodiscard]] scale_type scale() const {
        return m_scale;
    }

    void scale(scale_type const& scale) {
        this->m_scale = scale;
        update_matrix();
    }

    [[nodiscard]] rotation_type rotation() const {
        return m_rotation;
    }

    void rotation(rotation_type const& rotation) {
        this->m_rotation = rotation;
        update_matrix();
    }
};

using Transform2d = Transform<glm::vec2, glm::vec2, float>;
using Transform3d = Transform<glm::vec3, glm::vec3, glm::quat>;
