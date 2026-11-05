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
#include "DataTypes.h"
#include "game_components/EntityFactory.h"
#include "game_components/Entity.h"

struct DoorFieldIds {
    std::uint32_t unlocked_with{};
};

class DoorEntity final : public Entity {
    friend class DoorEntityFactory;
    friend class DoorEntityEditorFactory;
    components::Enum unlocked_by;

    public:
        explicit DoorEntity(std::uint32_t uid, uuid const& instance_id, glm::vec4 colour, components::Enum unlocked_by)
            : Entity(uid, instance_id, colour), unlocked_by(std::move(unlocked_by)) {
        }

        ~DoorEntity() override = default;

        void debug_log() override;
};

class DoorEntityEditorFactory : public TypeEntityEditorFactory<DoorEntity, DoorFieldIds> {
    public:
        DoorEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container,
            std::shared_ptr<DoorEntity> const& entity_instance);

        // explicit DoorEntityEditorFactory(ldtk::EntityDef const& entity_def);
        void save() override;
};

class DoorEntityFactory : public EntityFactory {
    // std::string unlocked_by_field_name;
    DoorFieldIds m_ids;
    public:
        explicit DoorEntityFactory(std::uint32_t const& entity_def_uid)
            : EntityFactory(entity_def_uid) {
        }

        ~DoorEntityFactory() override = default;
        void register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) override;

        std::unique_ptr<EntityEditorFactory> get_editor(std::shared_ptr<Entity> entity_instance) override;

        std::pair<uuid, std::shared_ptr<Entity>>
        instantiate(EntityContext& ctx, ldtk::EntityInstance const& entity_instance) override;
};