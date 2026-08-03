#include "engine/resources/texturestore.h"

#include <ranges>
#include <algorithm>
#include <string>
using namespace  std::string_literals;

uuid TextureStore::register_texture(const std::string& identifier, std::shared_ptr<Texture> texture) {
    uuid const uuuid = uuid5(TEXTURESTORE_NS, identifier);
    m_textures.emplace(uuuid, texture);
    return uuuid;
}

uuid TextureStore::register_texture(const uuid& identifier, std::shared_ptr<Texture> texture) {
    uuid const uuuid = uuid5(TEXTURESTORE_NS, identifier);
    m_textures.emplace(uuuid, texture);
    return uuuid;
}

std::shared_ptr<Texture> TextureStore::get_texture(const uuid uuid) {
    auto it = m_textures.find(uuid);
    if (it != m_textures.end()) {
        return it->second;
    }
    throw texture_not_registered("Texture not registered"s, uuid);
}