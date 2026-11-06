#pragma once
#include <cstdint>
#include <uuid.h>
#include <vector>
#include <entt/entity/fwd.hpp>
#include <glm/vec4.hpp>
#include <game_components/DataTypes.h>

struct LDtkEntityComponent {
    LDtkEntityComponent(std::uint32_t uid, uuid const& instance_id, glm::vec4 const& colour)
        : uid(uid),
          instance_id(instance_id),
          colour(colour) {
    }

    std::uint32_t uid;
    uuid instance_id;
    glm::vec4 colour;
};

struct ButtonComponent : public LDtkEntityComponent {
    std::vector<entt::entity> targets;
};

struct DoorComponent : public LDtkEntityComponent {
    DoorComponent(std::uint32_t uid, uuid const& instance_id, glm::vec4 const& colour,
        components::Enum const& unlocked_by)
        : LDtkEntityComponent(uid, instance_id, colour),
          unlocked_by(unlocked_by) {
    }

    components::Enum unlocked_by;
};

struct PlayerComponent : public LDtkEntityComponent {
    PlayerComponent(std::uint32_t uid, uuid const& instance_id, glm::vec4 const& colour, std::int32_t life,
        std::int32_t ammo)
        : LDtkEntityComponent(uid, instance_id, colour),
          life(life),
          ammo(ammo) {
    }

    std::int32_t life;
    std::int32_t ammo;
};