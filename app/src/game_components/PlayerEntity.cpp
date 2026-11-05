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

#include "game_components/PlayerEntity.h"

#include <engine/elog.h>
#include <print>
#include <iostream>

void PlayerEntity::debug_log() {
    std::println(elogDebug(), "PlayerEntity[{0::x}]{{ammo={1},life={2}}}", instance_id, ammo(), life());
}


PlayerEntityEditorFactory::PlayerEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container,
    std::shared_ptr<PlayerEntity> entity_instance): TypeEntityEditorFactory(ids, container, std::move(entity_instance)) {
    get_field<int32_t>(m_ids.ammo) = getInstance()->m_ammo;
    get_field<int32_t>(m_ids.life) = getInstance()->m_life;
    m_old = static_cast<EntityFieldContainer&>(*this);
    // field.emplace(getInstance().ammo());
}

void PlayerEntityEditorFactory::save() {
    // TODO: enumerate entity return vector of field and value
        instance_ptr pe = getInstance();
    if (pe) {
        // FIXME: field should match on uid instead of name.
        auto const& ammo = get_field<int32_t>(m_ids.ammo);
        if (ammo) {
            std::println(elogInfo(), "Changing 'Ammo':'{}' from '{}' to '{}'", ammo.name(), pe->ammo(), *ammo);
            pe->m_ammo = *ammo;
        }
        auto const& life = (get_field<int32_t>(m_ids.life));
        if (life) {
            std::println(elogInfo(), "Changing 'Life':'{}' from '{}' to '{}'", life.name(), pe->life(), *life);
            pe->m_life = *life;
        }
    } else {
        std::println(elogCritical(), "Provided Entity was not a PlayerEntity");
        std::terminate();
    }
}

void PlayerEntityFactory::register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) {
    EntityFactory::register_entity_def(ctx, entity_def);
    auto const& fields = entity_def.field_defs;

    // FIXME: This can be very prone to error if the ordering changes at all this will break.
    // FIXME: Modify LDtkLoader to expose uid for fields.
    for (auto const& [uid, field] : fields) {
        if (field->name == "ammo") {
            std::uint32_t id = m_ids.ammo = uid;

            // m_ids.ammo = field.uid;
            std::println(elogInfo(), "Found Field 'Life': \"{}\" ({})", field->name, id);
            m_container.add_field<int32_t>(field->name, id);

        }
        if (field->name == "life") {
            std::uint32_t id = m_ids.life = uid;
            std::println(elogInfo(), "Found Field 'Ammo': \"{}\" ({})", field->name, id);
            m_container.add_field<int32_t>(field->name, id);


        }
    }
    // auto const& life_field = fields[0];
    // life_field_name = life_field.name;
    // auto const& ammo_field = fields[1];
    // ammo_field_name = ammo_field.name;
    // m_editor = std::make_shared<PlayerEntityEditorFactory>(entity_def);
}

std::unique_ptr<EntityEditorFactory> PlayerEntityFactory::get_editor(std::shared_ptr<Entity> entity_instance)  {
    return std::make_unique<PlayerEntityEditorFactory>(m_ids, m_container, std::dynamic_pointer_cast<PlayerEntity>(entity_instance));
}

std::pair<uuid, std::shared_ptr<Entity>> PlayerEntityFactory::instantiate(EntityContext& ctx, ldtk::EntityInstance const& entity_instance) {
    auto const& ammo_it = std::ranges::find_if(entity_instance.fields, [id=m_ids.ammo](auto const& field) {
        return field->def_uid == id and field->type == ldtk::FieldType::Int;

    });
    auto const& fammo = *ammo_it;
    auto const& ammo_field = dynamic_cast<ldtk::getFieldInstanceType<ldtk::FieldType::Int> const&>(*fammo);
    // auto const& ammo_field = entity_instance.getField<ldtk::FieldType::Int>(m_ids.ammo);
    auto const& life_it = std::ranges::find_if(entity_instance.fields, [id=m_ids.life](auto const& field) {
        return field->def_uid == id and field->type == ldtk::FieldType::Int;

    });
    auto const& flife = *life_it;
    auto const& life_field = dynamic_cast<ldtk::getFieldInstanceType<ldtk::FieldType::Int> const&>(*flife);

    // auto const& life_field = entity_instance.getField<ldtk::FieldType::Int>(m_ids.life);
    // auto const& color = entity_instance.getColor();
    // auto const colour = glm::vec4{color.r, color.g, color.b, color.a};
    glm::vec4 colour = glm::vec4(1);
    if (ammo_field.value and life_field.value) {
        auto const& ammo = ammo_field.value.value();
        auto const& life = life_field.value.value();
        auto pe = std::make_shared<PlayerEntity>(type_id(), entity_instance.iid, colour, life, ammo);
        pe->transform = this->get_transform2d(entity_instance);
        return std::make_pair(pe->instance_id, pe);

    }
    return {{}, nullptr};
}
