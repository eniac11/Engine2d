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
#include <string>
#include <vector>
#include <algorithm>
#include <ranges>
#include <memory>

#include <ldtk_parser/defs/enum_def.h>

// #include "../../../engine/e2d/vendor/LDtkLoader/include/LDtkLoader/Enum.hpp"

namespace components {
    /// Shared Control block for valid enum values
    struct EnumTypeControlBlock {
        std::uint32_t uid;
        std::string type_name;
        std::vector<ldtk::EnumValue> valid_values{};


        bool valid_value(ldtk::EnumValue const& value) const {
            return std::ranges::contains(valid_values, value);
        }
    };

    struct Enum {
        /// Shared type information
        std::shared_ptr<EnumTypeControlBlock> e_type;
        ldtk::EnumValue const* enum_value;

        [[nodiscard]] bool empty() const {
            if (e_type) {
                return e_type->type_name.empty() and enum_value == nullptr;
            }
            return true;
        }

        std::string const& type_name() const {
            return e_type->type_name;
        }

        auto operator<=>(Enum const& other) const {
            if (e_type and other.e_type) {
                if (auto const cmp = e_type->type_name <=> other.e_type->type_name; cmp != 0 ) return cmp;

            }
            return enum_value <=> other.enum_value;

        }
        bool operator==(Enum const&) const = default;
    };
} // namespace components
