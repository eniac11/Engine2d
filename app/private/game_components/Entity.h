// 2d zero-code game engine
// Copyright (C) 2026. Hadley Epstein
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once
#include <engine/math.h>
// #include <engine/uuid.h>
#include <uuid.h>
#include <memory>

class EntityFactory;

class Entity {
    public:
        Transform2d transform;
        explicit Entity(std::uint32_t uid, uuid const& instance_id, glm::vec4 const& colour)
            : uid(uid), instance_id(instance_id), colour(colour) {
        }

        virtual ~Entity() = default;

        [[nodiscard]] glm::vec4 get_colour() const {
            return colour;
        }

        /// Returns the id of an instance of an entity definition type
        [[nodiscard]] uuid const& get_instance_id() const noexcept {
            return instance_id;
        }

        /// Returns the id of an instance of an entity definition type
        [[nodiscard]] std::uint32_t const& get_uid() const noexcept {
            return uid;
        }

        virtual void debug_log() = 0;
    protected:
        std::uint32_t uid;
        uuid instance_id;
        glm::vec4 colour;
};

struct EntityReference {
    uuid instance_id;
    std::weak_ptr<Entity> instance_entity;
    uuid ref_id;
    std::weak_ptr<Entity> ref_entity;
};

