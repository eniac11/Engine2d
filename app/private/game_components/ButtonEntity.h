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
#include "game_components/EntityFactory.h"
#include "game_components/Entity.h"

struct ButtonEntityIds {
    uint32_t targets{};
};

class ButtonEntity final : public Entity {
    friend class ButtonEntityFactory;
    friend class ButtonEntityEditorFactory;
    std::vector<std::shared_ptr<EntityReference>> m_targets;
    public:
        explicit ButtonEntity(std::uint32_t uid, uuid const& instance_id, glm::vec4 colour)
            : Entity(uid, instance_id, colour) {
        }

        ~ButtonEntity() override = default;

        void debug_log() override;
};

class ButtonEntityEditorFactory : public TypeEntityEditorFactory<ButtonEntity, ButtonEntityIds> {
    // ButtonEntityIds const& m_ids;
    public:
        ButtonEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container,
            std::shared_ptr<ButtonEntity> const& entity_instance);

        void save() override;

        // explicit ButtonEntityEditorFactory(ldtk::EntityDef const& entity_def);
        //
        // std::vector<EntityField> const& default_fields() const override;
        // std::vector<EntityField> enumerate_fields(Entity const& entity) override;
        // void set_field(Entity& entity, EntityField const& field) override;
};

class ButtonEntityFactory : public EntityFactory {
    ButtonEntityIds m_ids;
    // EntityFieldContainer m_container;
    public:
        explicit ButtonEntityFactory(std::uint32_t const& entity_def_uid)
            : EntityFactory(entity_def_uid) {
        }

        ~ButtonEntityFactory() override = default;
        void register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) override;

        std::unique_ptr<EntityEditorFactory> get_editor(std::shared_ptr<Entity> entity_instance) override;

        std::pair<uuid, std::shared_ptr<Entity>>
        instantiate(EntityContext& ctx, ldtk::EntityInstance const& entity_instance) override;

};