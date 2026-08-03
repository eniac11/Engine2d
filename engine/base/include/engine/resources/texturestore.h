#pragma once
#include <map>
#include <memory>
#include <vector>

#include "../graphics/Texture.h"
#include "engine/uuid.h"

// uuid v7
constexpr uuid TEXTURESTORE_NS = {0x01, 0x9c, 0x00, 0xce, 0xe3, 0x93, 0x75, 0xa6, 0xa4, 0x62, 0x61, 0x6d, 0x1b, 0x52, 0x51, 0x4a};

class texture_not_registered : public texture_exception {
    public:
        texture_not_registered(const std::string &basic_string, uuid uuuid)
            : texture_exception(std::format("{0}: {1}", basic_string, uuuid)) {

        }
};

class TextureStore {
    std::map<uuid, std::shared_ptr<Texture> > m_textures;

    public:
        uuid register_texture(const std::string& identifier, std::shared_ptr<Texture> texture);
        uuid register_texture(const uuid& identifier, std::shared_ptr<Texture> texture);
        std::shared_ptr<Texture> get_texture(uuid const uuid);
};
