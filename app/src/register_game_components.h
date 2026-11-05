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
#include <uuid.h>
#include "game_components/EntityFactoryRegistry.h"
#include "game_components/EntityFieldTypeRegistry.h"

constexpr uuid PLAYER_ENTITY_TYPE = {1, 160, 128, 96, 68, 94, 112, 0, 141, 135, 220, 24, 119, 13, 129, 207};

void register_game_components(EntityFactoryRegistry& registry);
void register_entity_field_types(EntityFieldTypeRegistry& registry);
