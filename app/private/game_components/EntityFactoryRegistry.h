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

#include <uuid.h>

#include <ldtk_parser/entity.h>

#include <map>
#include <memory>


class EntityFactoryRegistry {
    std::map<std::uint32_t, std::unique_ptr<EntityFactory>> m_entity_factories;

    public:
        void register_factory(std::unique_ptr<EntityFactory>&& factory);

        void register_defs(EntityContext& ctx, ldtk::Project const& project);

        std::vector<uint32_t> get_factory_ids();
        EntityFactory& get_factory(uint32_t factory_id);

        std::pair<uuid, std::shared_ptr<Entity>> instantiate_entity(EntityContext& ctx,
                                                                    uint32_t const& entity_type, ldtk::EntityInstance const& entity_instance);
};
