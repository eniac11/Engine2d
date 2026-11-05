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

#include "register_game_components.h"

#include <game_components/EntityField.h>

#include <game_components/ButtonEntity.h>
#include <game_components/DoorEntity.h>
#include <game_components/PlayerEntity.h>
#include <imgui/imgui.h>

void register_game_components(EntityFactoryRegistry& registry) {
    registry.register_factory(std::make_unique<PlayerEntityFactory>(59));
    registry.register_factory(std::make_unique<ButtonEntityFactory>(105));
    registry.register_factory(std::make_unique<DoorEntityFactory>(62));
}



class IntegerRenderer : public TypedEntityPropertyRender<int32_t> {
    public:
        explicit IntegerRenderer(std::type_index const& index)
            : TypedEntityPropertyRender(index) {
        }

        void render_field(EntityField<int32_t>& field) override {
            ImGui::TextUnformatted(field.name().c_str());
            ImGui::Text("%i", field.uid());
            if (field) {
                auto value = *field;
                if (ImGui::InputInt(field.name().c_str(), &value)) {
                    field = value;
                }

            }

        }
};

class EnumRenderer : public TypedEntityPropertyRender<components::Enum> {
    public:
        explicit EnumRenderer(std::type_index const& index)
            : TypedEntityPropertyRender(index) {
        }

        void render_field(EntityField<components::Enum>& field) override {
            ImGui::TextUnformatted(field.name().c_str());
            ImGui::Text("%i", field.uid());
            if (field) {
                auto& value = *field;
                const ldtk::EnumValue* current_item = value.enum_value;
                const char* s = nullptr;
                if (value.enum_value) {
                    s = value.enum_value->id.c_str();
                }
                if (ImGui::BeginCombo(value.e_type->type_name.c_str(), s)) {
                    for (auto const& evalue : value.e_type->valid_values) {
                        bool selected = &evalue == current_item;
                        if (ImGui::Selectable(evalue.id.c_str(), selected) and &evalue != current_item) {
                            value.enum_value = &evalue;
                        }

                    }
                    ImGui::EndCombo();
                }

            }

        }
};

void register_entity_field_types(EntityFieldTypeRegistry& registry) {
    registry.register_field<int32_t, IntegerRenderer>();
    registry.register_field<components::Enum, EnumRenderer>();
}