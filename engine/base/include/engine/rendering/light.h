#pragma once
#include <glm/vec3.hpp>

struct Light {
    glm::vec3 position;
    // float _pad0;
    glm::vec3 ambient;
    // float _pad1;
    glm::vec3 diffuse;
    // float _pad2;
    glm::vec3 specular;
};
