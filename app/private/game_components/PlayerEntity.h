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
#include "EntityFactory.h"
#include "game_components/Entity.h"



struct PlayerFieldIds {
    std::uint32_t ammo{};
    std::uint32_t life{};
};

class PlayerEntityFactory;

class PlayerEntity final : public Entity {
    friend class PlayerEntityFactory;
    friend class PlayerEntityEditorFactory;
    std::int32_t m_life;
    std::int32_t m_ammo;

    public:
        PlayerEntity(std::uint32_t uid, uuid const& instance_id, glm::vec4 colour, std::int32_t life, std::int32_t ammo)
            : Entity(uid, instance_id, colour),
              m_life(life),
              m_ammo(ammo) {
        }

        ~PlayerEntity() override = default;

        void debug_log() override;

        [[nodiscard]] std::int32_t ammo() const {
            return m_ammo;
        }

        [[nodiscard]] std::int32_t life() const {
            return m_life;
        }

        void set_life(std::int32_t life) {
            m_life = life;
        }


        void set_ammo(std::int32_t ammo) {
            m_ammo = ammo;
        }
};

class PlayerEntityEditorFactory : public TypeEntityEditorFactory<PlayerEntity, PlayerFieldIds> {
    // PlayerFieldIds const& m_ids;
    public:
        PlayerEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container, std::shared_ptr<PlayerEntity> entity_instance);

        void save() override;

        // std::vector<EntityField> const& default_fields() const override;
        // std::vector<EntityField> enumerate_fields(Entity const& entity) override;
        // void set_field(Entity& entity, EntityField const& field) override;
};


class PlayerEntityFactory final : public EntityFactory {
    PlayerFieldIds m_ids;

    public:
        explicit PlayerEntityFactory(std::uint32_t const& entity_def_uid)
            : EntityFactory(entity_def_uid) {
        }

        ~PlayerEntityFactory() override = default;

        void register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) override;

        std::unique_ptr<EntityEditorFactory> get_editor(std::shared_ptr<Entity> entity_instance)  override;


        std::pair<uuid, std::shared_ptr<Entity>> instantiate(EntityContext& ctx, ldtk::EntityInstance const& entity_instance) override;
};