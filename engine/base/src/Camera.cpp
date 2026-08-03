
#include "engine/Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/io.hpp>
#include <iostream>

#include "engine_p/logging_categories.h"

Camera::Camera() : m_position(0.f, 0.f, 0.f), m_forward(0.0f, 0.0f, -1.0f), m_view(1.0) {}

Camera::Camera(const glm::vec3 position) : m_position(position), m_forward(0.0f, 0.0f, -1.0f), m_view(1.0) {}

Camera::Camera(const glm::vec3 position, const glm::vec3 direction) :
    m_position(position), m_forward(direction), m_view(1.0) {}

float Camera::near_plane() const { return m_near; }

void Camera::set_near_plane(const float near) {
    m_near = near;
    update_projection();
}

float Camera::far_plane() const { return m_far; }

void Camera::set_far_plane(const float far) {
    m_far = far;
    update_projection();
}


glm::vec3 Camera::position() const { return m_position; }

glm::vec3 Camera::direction() const { return m_forward; }

void Camera::set_position(const glm::vec3 position) {
    m_position = position;
    update_view();
}

void Camera::set_direction(const glm::vec3 direction) {
    m_forward = direction;
    update_view();
}

glm::mat4 Camera::view() const { return m_view; }

glm::mat4 Camera::projection() const { return m_projection; }

void Camera::look_at(const glm::vec3 target) { m_view = glm::lookAt(m_position, target, UP); }

void Camera::update_view() { m_view = glm::lookAt(m_position, m_position + m_forward, UP); }


PerspectiveCamera::PerspectiveCamera(glm::vec3 position) {}

PerspectiveCamera::PerspectiveCamera(glm::vec3 position, glm::vec3 direction) {}

float PerspectiveCamera::fov() const { return m_fov; }

void PerspectiveCamera::set_fov(const float fov) {
    m_fov = fov;
    update_projection();
}

float PerspectiveCamera::aspect() const { return m_aspect; }

void PerspectiveCamera::set_aspect(float aspect) {
    m_aspect = aspect;
    update_projection();
}

void PerspectiveCamera::update_projection() {
    m_projection = glm::perspective(m_fov, aspect(), near_plane(), far_plane());
}

OrthographicCamera::OrthographicCamera() : m_bounds({0, 300, 0, 300}) {}

OrthographicCamera::OrthographicCamera(glm::vec4 bounds) : Camera(), m_bounds(bounds) { update_projection(); }

OrthographicCamera::OrthographicCamera(float width, float height) : Camera(), m_bounds({0.0f, width, height, 0.0f}) {
    update_projection();
}

OrthographicCamera::OrthographicCamera(glm::vec4 bounds, glm::vec3 position) : Camera(position), m_bounds(bounds) {
    update_projection();
}

OrthographicCamera::OrthographicCamera(float width, float height, glm::vec3 position) :
    Camera(position), m_bounds({0.0f, width, height, 0.0f}) {}

OrthographicCamera::OrthographicCamera(glm::vec4 bounds, glm::vec3 position, glm::vec3 direction) :
    Camera(position, direction), m_bounds(bounds) {
    update_projection();
}

OrthographicCamera::OrthographicCamera(float width, float height, glm::vec3 position, glm::vec3 direction) :
    Camera(position, direction), m_bounds({0.0f, width, height, 0.0f}) {
    update_projection();
}

glm::vec4 OrthographicCamera::bounds() const { return m_bounds; }

void OrthographicCamera::set_bounds(const glm::vec4 bounds) {
    m_bounds = bounds;
    update_projection();
}


void OrthographicCamera::update_projection() {
    constexpr std::size_t bounds_left_idx = 0;
    constexpr std::size_t bounds_right_idx = 1;
    constexpr std::size_t bounds_bottom_idx = 2;
    constexpr std::size_t bounds_top_idx = 3;
    elogCDebugEnabled(lcEngineGraphics) elogCDebug(lcEngineGraphics) << m_bounds << std::endl;
    m_projection = glm::ortho(m_bounds[bounds_left_idx], m_bounds[bounds_right_idx], m_bounds[bounds_bottom_idx],
                              m_bounds[bounds_top_idx], near_plane(), far_plane());
    elogCDebugEnabled(lcEngineGraphics) elogCDebug(lcEngineGraphics) << m_projection << std::endl;
}
