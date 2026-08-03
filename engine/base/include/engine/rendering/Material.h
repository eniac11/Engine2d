#pragma once
#include <memory>
#include <vector>
#include <glm/vec3.hpp>

#include "engine/Texture.h"

 struct Material {
    // std::vector<std::shared_ptr<Texture>> diffuse_maps;
    // std::vector<std::shared_ptr<Texture>> specular_maps;

    glm::vec3 ambient{1.0f};
    // float _pad0;
    glm::vec3 diffuse{1.0f};
    // float _pad1;
    glm::highp_vec3 specular{0.0f};
     float shininess{0.0f};
};
