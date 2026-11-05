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
#include <memory>
// #include <engine/uuid.h>
#include <game_components/DataTypes.h>

#include <ldtk_parser/entity.h>
#include <ldtk_parser/defs/enum_def.h>
#include <ldtk_parser/defs/entity_def.h>


#include "EntityField.h"
#include "game_components/Entity.h"

// #include <ldtk>

// struct EntityField {
//     std::string name;
//     ldtk::FieldType type;
//     std::uint32_t uid;
// };

class EntityContext {
    std::vector<std::shared_ptr<EntityReference>> entity_references;
    std::map<std::uint32_t, std::shared_ptr<components::EnumTypeControlBlock>> m_enums;
    public:
        std::shared_ptr<EntityReference> add_entity_reference(std::shared_ptr<Entity> entity, uuid ref_id);
        void add_enum_def(ldtk::EnumDef const& enum_);

        void resolve_entity_reference(std::shared_ptr<Entity> ref_entity);
        std::shared_ptr<components::EnumTypeControlBlock> resolve_enum_def(uint32_t uid);
};

class EntityEditorFactory : public EntityFieldContainer {
    protected:
        std::shared_ptr<Entity> instance;
        EntityFieldContainer m_old;
    public:
        explicit EntityEditorFactory(EntityFieldContainer const& container, std::shared_ptr<Entity> entity_instance)
        // Explicitly copying here
            : EntityFieldContainer(container), instance(std::move(entity_instance)) {
        }
        // virtual ~EntityEditorFactory() = default;
        virtual void save() = 0;
        bool needs_saving() {
            for (auto const& [this_field, old_field] : std::views::zip(enumerate_fields(), m_old.enumerate_fields())) {
                auto const& field = get_field(this_field);
                auto const& ofield = m_old.get_field(old_field);
                if (field.changed(ofield)) {
                    return true;
                }
            }
            return false;
        }

        // virtual std::vector<EntityField> const& default_fields() const = 0;
        // virtual std::vector<EntityField> enumerate_fields(Entity const& entity) = 0;
        // virtual void set_field(Entity& entity, EntityField const& field) = 0;
};

template <typename T, typename IDs_type>
class TypeEntityEditorFactory : public EntityEditorFactory {
    public:
        using ids_type = IDs_type;
    protected:
        ids_type const& m_ids;
    public:
        using instance_type = T;
        using instance_ptr = std::shared_ptr<T>;
        TypeEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container, std::shared_ptr<T> const& entity_instance)
            : EntityEditorFactory(container, entity_instance), m_ids(ids) {
        }

    protected:
        instance_ptr getInstance() {
            return std::dynamic_pointer_cast<instance_type>(instance);
        }
};



class EntityFactory {
    std::uint32_t entity_def_uid;
    std::string m_name{};

    protected:
        // std::shared_ptr<EntityEditorFactory> m_editor;
        EntityFieldContainer m_container;

    public:
        explicit EntityFactory(std::uint32_t const& entity_def_uid)
            : entity_def_uid(entity_def_uid) {
        }

        virtual ~EntityFactory() = default;

        virtual void register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) {
            m_name = entity_def.name;
            // m_container = EntityFieldContainer::build(entity_def);
        }

        [[nodiscard]] std::uint32_t type_id() const {
            return entity_def_uid;
        }

        std::string name() const {
            return m_name;
        }

        virtual std::unique_ptr<EntityEditorFactory> get_editor(std::shared_ptr<Entity> entity_instance) = 0;

        virtual std::pair<uuid, std::shared_ptr<Entity>> instantiate(EntityContext& ctx,
                                                                     ldtk::EntityInstance const& entity_instance) = 0;

    protected:
        Transform2d get_transform2d(ldtk::EntityInstance const& entity_instance);
};
