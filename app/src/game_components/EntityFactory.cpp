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

#include "game_components/EntityFactory.h"

#include <engine/elog.h>
#include <game_components/ButtonEntity.h>
#include <game_components/ButtonEntity.h>

std::shared_ptr<EntityReference> EntityContext::add_entity_reference(std::shared_ptr<Entity> entity, uuid ref_id) {
    uuid id = entity->get_instance_id();
    std::println(elogInfo(), "Requesting EntityReference for {::x} refering to {::x}", id, ref_id);
    return entity_references.emplace_back(std::make_shared<EntityReference>(id, entity, ref_id));
}

void EntityContext::resolve_entity_reference(std::shared_ptr<Entity> ref_entity) {
    for (auto& ref : entity_references) {
        if (ref->ref_id == ref_entity->get_instance_id()) {
            std::println(elogInfo(), "Resolving EntityReference for {::x}: reference entity {::x}", ref->instance_id, ref_entity->get_instance_id());
            ref->ref_entity = ref_entity;
        }
    }
}

std::shared_ptr<components::EnumTypeControlBlock> EntityContext::resolve_enum_def(uint32_t uid) {
    return m_enums[uid];
}

void EntityContext::add_enum_def(ldtk::EnumDef const& enum_) {
    std::println(elogInfo(), "Found Enum '{}': '{}", enum_.name, enum_.uid);
    auto cb = std::make_shared<components::EnumTypeControlBlock>(enum_.uid, enum_.name);
    cb->valid_values.append_range(enum_.values | std::views::values);
    m_enums[enum_.uid] = cb;
}

Transform2d EntityFactory::get_transform2d(ldtk::EntityInstance const& entity_instance) {
    Transform2d transform2d;
    glm::ivec2 const& pos = entity_instance.grid_position;
    transform2d.position(pos*glm::ivec2{16, 16});
    transform2d.scale({16, 16});
    return transform2d;
}
