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
#include "game_components/EntityField.h"
#include <memory>
#include <map>
#include <typeindex>
#include <imgui/imgui.h>
#include <print>
#include <engine/elog.h>
#include <iostream>

class IEntityPropertyRenderer {
   std::type_index type_index;
    public:
       explicit IEntityPropertyRenderer(std::type_index index) : type_index(index) {};
        virtual ~IEntityPropertyRenderer() = default;

        [[nodiscard]] std::type_index get_type_index() const {
            return type_index;
        }

        virtual void render_field(IEntityField& field) = 0;
};

template <typename T>
class TypedEntityPropertyRender : public IEntityPropertyRenderer {
    public:
        explicit TypedEntityPropertyRender(std::type_index const& index)
            : IEntityPropertyRenderer(index) {
        }


        void render_field(IEntityField& field) final {
            try {
                auto& casted_field = dynamic_cast<EntityField<T>&>(field);
                render_field(casted_field);
            } catch (std::bad_cast const& e) {
                return;
            }
        }

        virtual void render_field(EntityField<T>& field) = 0;
};

class EntityFieldTypeRegistry {
    std::map<std::type_index, std::unique_ptr<IEntityPropertyRenderer>> m_field_renderers;
    public:
        template <typename T, std::derived_from<TypedEntityPropertyRender<T>> TRenderer, std::derived_from<EntityField<T>> TField=EntityField<T>>
        void register_field() {
            std::println(elogInfo(), "Registry Field Type: {} with Renderer {}", typeid(TField).name(), typeid(TRenderer).name());
            auto index = std::type_index(typeid(TField));
            std::cout << index.name() << std::endl;
            m_field_renderers[index] = std::unique_ptr<IEntityPropertyRenderer>(new TRenderer(index));
        }

        void render_field(IEntityField& field) {
            auto const& index = field.get_type_index();
            auto it = m_field_renderers.find(index);
            if (it != m_field_renderers.end()) {
                m_field_renderers[index]->render_field(field);
                return;
            }
            ImGui::Text("UNKNOWN: %s (%i)", field.name().c_str(), field.uid());
        }
};