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

#include "game_components/EntityField.h"
// #include "game_components/Entity.h"

#include <c++/14/filesystem>
#include <engine/elog.h>
#include <print>

#include "game_components/DataTypes.h"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

void EntityFieldContainer::add_fields_from_entity_def(ldtk::EntityDef const& def) {
    constexpr auto null = std::nullopt;
    for (auto const& [uid, field] : def.field_defs) {
        // std::uint32_t uid = uid;
        switch (field->type) {
            case ldtk::FieldType::Int:
                add_field<int>(field->name, uid, def.get_field<ldtk::FieldType::Int>(uid).default_override);
                return;
            case ldtk::FieldType::Float:
                add_field<float>(field->name, uid, def.get_field<ldtk::FieldType::Float>(uid).default_override);
                return;
            case ldtk::FieldType::Bool:
                add_field<bool>(field->name, uid, def.get_field<ldtk::FieldType::Bool>(uid).default_override);
                return;
            case ldtk::FieldType::String:
                add_field<std::string>(field->name, uid, def.get_field<ldtk::FieldType::String>(uid).default_override);
                return;
            // case ldtk::FieldType::Color:
            //     add_field<glm::vec4>(field->name, uid, null);
            case ldtk::FieldType::Point:
                add_field<glm::vec2>(field->name, uid, null);
                return;
            case ldtk::FieldType::Enum:
                add_field<components::Enum>(field->name, uid, null);
                return;
            // case ldtk::FieldType::Pat:
                // add_field<std::filesystem::path>(field->name, uid, null);
            case ldtk::FieldType::EntityRef:
                // add_field<EntityReference>(field->name, uid, null);
            case ldtk::FieldType::Tile:
                // break;
            // case ldtk::FieldType::ArrayInt:
            //     // break;
            // case ldtk::FieldType::ArrayFloat:
            //     // break;
            // case ldtk::FieldType::ArrayBool:
            //     // break;
            // case ldtk::FieldType::ArrayString:
            //     // break;
            // case ldtk::FieldType::ArrayColor:
            //     // break;
            // case ldtk::FieldType::ArrayPoint:
            //     // break;
            // case ldtk::FieldType::ArrayEnum:
            //     // break;
            // case ldtk::FieldType::ArrayFilePath:
            //     // break;
            // case ldtk::FieldType::ArrayTile:
            //     // break;
            // case ldtk::FieldType::ArrayEntityRef:
                {
                    std::println(elogDebug(), "TODO: implement Array Fields and Tile");
                }
                // break;
                continue;
        }
    }
}
