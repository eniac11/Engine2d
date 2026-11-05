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

#include "game_components/DoorEntity.h"
#include <ldtk_parser/defs/entity_def.h>

#include <engine/elog.h>

void DoorEntity::debug_log() {
    std::string unlock{};
    if (!unlocked_by.empty() and unlocked_by.enum_value) {
        unlock = std::format("{}.{}", unlocked_by.type_name(), unlocked_by.enum_value->id);
    }
    std::println(elogDebug(), "DoorEntity[{0::x}]{{unlocked_by={1}}}", instance_id, unlock);
}

DoorEntityEditorFactory::DoorEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container,
    std::shared_ptr<DoorEntity> const& entity_instance): TypeEntityEditorFactory(ids, container, entity_instance) {
    get_field<components::Enum>(m_ids.unlocked_with) = getInstance()->unlocked_by;
    m_old = static_cast<EntityFieldContainer&>(*this);
}

void DoorEntityEditorFactory::save() {
    // TODO: enumerate entity return vector of field and value
    instance_ptr pe = getInstance();
    if (pe) {
        // FIXME: field should match on uid instead of name.
        auto const& unlocked_with = get_field<components::Enum>(m_ids.unlocked_with);
        if (unlocked_with) {
            std::println(elogInfo(), "Changing 'UnlockedWith':'{0}' from '{2}.{1}' to '{2}.{3}'", unlocked_with.name(), pe->unlocked_by.enum_value ? pe->unlocked_by.enum_value->id : "None", pe->unlocked_by.type_name(), unlocked_with->enum_value? unlocked_with->enum_value->id : "None");
            pe->unlocked_by = *unlocked_with;
        }

    } else {
        std::println(elogCritical(), "Provided Entity was not a DoorEntity");
        std::terminate();
    }
}



void DoorEntityFactory::register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) {
    EntityFactory::register_entity_def(ctx, entity_def);
    auto const& fields = entity_def.field_defs;
    // entity_def.get_field<ldtk::FieldType::Enum>()

    for (auto const& [uid, field] : fields) {

        if (field->name == "lockedWith" and field->type == ldtk::FieldType::Enum) {
            std::uint32_t id = m_ids.unlocked_with = uid;

            // m_ids.ammo = field.uid;
            std::println(elogInfo(), "Found Field 'unlocked_with': \"{}\" ({})", field->name, id);
            auto const& e_def = entity_def.get_field<ldtk::FieldType::Enum>(uid);

            auto e_type = ctx.resolve_enum_def(e_def.enum_def_uid);
            const ldtk::EnumValue* value = nullptr;
            if (e_def.default_override) {
                value = e_def.default_override->value;
            }

            m_container.add_field<components::Enum>(field->name, id, components::Enum(e_type, value));
            // m_container.get_field<components::Enum>(m_ids.unlocked_with)->e_type = e_type;
            // auto& values = m_container.get_field<components::Enum>(m_ids.unlocked_with)->e_type->valid_values;


        }

    }
}

std::unique_ptr<EntityEditorFactory> DoorEntityFactory::get_editor(std::shared_ptr<Entity> entity_instance) {
    return std::make_unique<DoorEntityEditorFactory>(m_ids, m_container, std::dynamic_pointer_cast<DoorEntity>(entity_instance));
}

std::pair<uuid, std::shared_ptr<Entity>> DoorEntityFactory::instantiate(EntityContext& ctx,
                                                                        ldtk::EntityInstance const& entity_instance) {
    // uuid id = uuid_string_to_uuid(entity_instance.iid);
    auto const& it = std::ranges::find_if(entity_instance.fields, [id=m_ids.unlocked_with](auto const& field) {
        return field->def_uid == id and field->type == ldtk::FieldType::Enum;

    });
    auto const& f = *it;
    auto const& unlocked_with_field = dynamic_cast<ldtk::getFieldInstanceType<ldtk::FieldType::Enum> const&>(*f);
    // [m_ids]. <ldtk::FieldType::Enum>(m_ids.unlocked_with);
    // auto const& color = entity_instance.getColor();
    // auto const colour = glm::vec4{color.r, color.g, color.b, color.a};
    auto const colour = glm::vec4(1);
    auto const& g = *unlocked_with_field.def;
    const auto& def =  dynamic_cast<ldtk::EnumFieldDef const&>(*g);

    components::Enum e{
    .e_type =ctx.resolve_enum_def(def.enum_def_uid),

    };


    if (unlocked_with_field.value) {
        auto const& value = unlocked_with_field.value.value();

        e.enum_value = value.value;
    }
    auto de = std::make_shared<DoorEntity>(type_id(), entity_instance.iid, colour, e);
    de->transform = this->get_transform2d(entity_instance);
    ctx.resolve_entity_reference(de);

    return {entity_instance.iid, de};
}
