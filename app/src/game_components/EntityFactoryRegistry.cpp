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

#include "game_components/EntityFactoryRegistry.h"

#include <ranges>
#include <engine/elog.h>
#include <ldtk_parser/project.h>

void EntityFactoryRegistry::register_factory(std::unique_ptr<EntityFactory>&& factory) {
    std::println(elogInfo(), "Registring EntityFactory: '{}'", factory->type_id());
    m_entity_factories[factory->type_id()] = std::move(factory);
}

void EntityFactoryRegistry::register_defs(EntityContext& ctx, ldtk::Project const& project) {
    std::println(elogInfo(), "Registering defs...");

    std::println(elogInfo(), "Registering enums...");

    for (auto const& enum_def : project.enum_defs) {
        ctx.add_enum_def(enum_def.second);
    }


    std::println(elogInfo(), "Registering entities...");
    for (auto& [key, factory] : m_entity_factories) {
        try {
            auto const& entity_def = project.entity_defs.at(key);
            std::println(elogInfo(), "Found Entity definition: {} name: '{}'", factory->type_id(), entity_def.name);

            factory->register_entity_def(ctx, entity_def);


        } catch (const std::invalid_argument& e) {
            std::println(elogError(), "Could not find entity def in project");
        }
    }
}

std::vector<uint32_t> EntityFactoryRegistry::get_factory_ids() {
    std::vector<std::uint32_t> factory_ids;
    factory_ids.reserve(m_entity_factories.size());
    for (auto const& key : m_entity_factories | std::views::keys) {
        factory_ids.push_back(key);
    }
    return factory_ids;
}

EntityFactory& EntityFactoryRegistry::get_factory(uint32_t factory_id) {
    auto factory = m_entity_factories.find(factory_id);
    if (factory == m_entity_factories.end()) {
        throw std::runtime_error("Could not find factory for entity type");
    }
    return *factory->second;
}

std::pair<uuid, std::shared_ptr<Entity>> EntityFactoryRegistry::instantiate_entity(
    EntityContext& ctx, uint32_t const& entity_type, ldtk::EntityInstance const& entity_instance) {
    auto& factory = get_factory(entity_type);
    return factory.instantiate(ctx, entity_instance);
}
