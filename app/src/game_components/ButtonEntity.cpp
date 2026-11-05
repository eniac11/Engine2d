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

#include "game_components/ButtonEntity.h"
#include <ldtk_parser/types.h>
#include <engine/elog.h>
#include <ldtk_parser/defs/entity_def.h>


void ButtonEntity::debug_log() {
    std::println(elogDebug(), "ButtonEntity[{0::x}] targets:", instance_id);
    for (auto const& ref : m_targets) {
        if (auto other = ref->ref_entity.lock()) {
            std::println(elogDebug(),"{::x}", other->get_instance_id());
        }
    }

}

ButtonEntityEditorFactory::ButtonEntityEditorFactory(ids_type const& ids, EntityFieldContainer const& container,
    std::shared_ptr<ButtonEntity> const& entity_instance): TypeEntityEditorFactory(ids, container, entity_instance) {
    std::println(elogInfo(), "TODO: Array fields get_field Array<EntityRef> 'targets'");
    // *get_field<int32_t>(m_ids.targets) = getInstance()->m_targets;
    m_old = static_cast<EntityFieldContainer&>(*this);
}

void ButtonEntityEditorFactory::save() {
    std::println(elogInfo(), "TODO: save Array fields get_field Array<EntityRef> 'targets'");
    instance_ptr pe = getInstance();
    if (pe) {
        // FIXME: field should match on uid instead of name.
        auto const& targets = get_field<int32_t>(m_ids.targets);
        if (targets) {
            std::println(elogInfo(), "Changing 'Targets':'{}' from '{}' to '{}'", targets.name(), pe->m_targets.size(), *targets);
        }

    } else {
        std::println(elogCritical(), "Provided Entity was not a ButtonEntity");
        std::terminate();
    }

}

void ButtonEntityFactory::register_entity_def(EntityContext& ctx, ldtk::EntityDef const& entity_def) {
    EntityFactory::register_entity_def(ctx, entity_def);
    auto const& fields = entity_def.field_defs;
    // FIXME: This can be very prone to error if the ordering changes at all this will break.
    // FIXME: Modify LDtkLoader to expose uid for fields.
    for (auto const& [uid, field] : fields) {
        if (field->name == "targets") {
            std::uint32_t id = m_ids.targets = field->uid;
            std::println(elogInfo(), "Found Field 'Targets': \"{}\"", field->name);

            std::println(elogInfo(), "TODO: Array Fields Array<EntityRef> 'targets'");
            // m_container.add_field<int32_t>(field.name, id);

        }

    }
    // targets_field_name = fields[0].name;
    // m_editor = std::make_shared<ButtonEntityEditorFactory>(entity_def);

}

std::unique_ptr<EntityEditorFactory> ButtonEntityFactory::get_editor(std::shared_ptr<Entity> entity_instance) {
    return std::make_unique<ButtonEntityEditorFactory>(m_ids, m_container, std::dynamic_pointer_cast<ButtonEntity>(entity_instance));
}

std::pair<uuid, std::shared_ptr<Entity>> ButtonEntityFactory::instantiate(EntityContext& ctx,
                                                                          ldtk::EntityInstance const& entity_instance) {
    // auto const& targets = entity_instance.getArrayField<ldtk::EntityRef>(m_ids.targets);

    // auto const& color = entity_instance.getColor();
    // auto const colour = glm::vec4{color.r, color.g, color.b, color.a};
    auto const colour = glm::vec4(1);
    auto be = std::make_shared<ButtonEntity>(type_id(),entity_instance.iid, colour);
    // for (auto const& target : targets) {
    //     if (!target.is_null()) {
    //         be->m_targets.push_back(ctx.add_entity_reference(be, uuid_string_to_uuid(target.value().entity_iid.str())));
    //     }
    // }
    ctx.resolve_entity_reference(be);
    be->transform = this->get_transform2d(entity_instance);
    return {be->get_instance_id(),be};
}
