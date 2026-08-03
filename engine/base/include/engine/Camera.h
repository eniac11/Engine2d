#pragma once

#include <glm/glm.hpp>

constexpr auto UP = glm::vec3(0.0f, 1.0f, 0.0f);


class Camera {

    public:
        float near_plane() const;
        void set_near_plane(float near);

        float far_plane() const;
        void set_far_plane(float far);


        glm::vec3 position() const;
        void set_position(glm::vec3 position);

        glm::vec3 direction() const;
        void set_direction(glm::vec3 direction);

        glm::mat4 view() const;
        glm::mat4 projection() const;
        void look_at(glm::vec3 target);
        void update_view();

    protected:
        Camera();
        virtual ~Camera() = default;
        explicit Camera(glm::vec3 position);
        Camera(glm::vec3 position, glm::vec3 direction);
        virtual void update_projection() = 0;
        glm::mat4 m_projection;
        glm::mat4 m_view;

    private:
        glm::vec3 m_forward;
        glm::vec3 m_position;

        float m_near = 0.1f;
        float m_far = 100.0f;
};


class PerspectiveCamera final : public Camera {
    public:
        PerspectiveCamera() = default;
        explicit PerspectiveCamera(glm::vec3 position);
        PerspectiveCamera(glm::vec3 position, glm::vec3 direction);
        float fov() const;
        void set_fov(float fov);

        float aspect() const;
        void set_aspect(float aspect);
        // Camera interface
    protected:
        void update_projection() override;

    private:
        float m_fov = glm::radians(45.0f);
        float m_aspect = 16 / 9;
};

class OrthographicCamera final : public Camera {
    public:
        OrthographicCamera();
        explicit OrthographicCamera(glm::vec4 bounds);
        OrthographicCamera(float width, float height);
        OrthographicCamera(glm::vec4 bounds, glm::vec3 position);
        OrthographicCamera(float width, float height, glm::vec3 position);
        OrthographicCamera(glm::vec4 bounds, glm::vec3 position, glm::vec3 direction);
        OrthographicCamera(float width, float height, glm::vec3 position, glm::vec3 direction);

        glm::vec4 bounds() const;
        void set_bounds(glm::vec4 bounds);

        // Camera interface
    protected:
        void update_projection() override;

    private:
        glm::vec4 m_bounds;
};

struct alignas(16) ShaderCamera {
        glm::mat4 projection{};
        glm::vec3 position{};
        // float _pad0{0.0f};
        glm::mat4 view{};

        explicit ShaderCamera(Camera& camera) {
            projection = camera.projection();
            position = camera.position();
            view = camera.view();
        }
};

