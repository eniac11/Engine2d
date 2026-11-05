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
#include <optional>
#include <string>
#include <cstdint>
#include <ranges>
#include <memory>
#include <ldtk_parser/defs/entity_def.h>
#include <ldtk_parser/defs/field_def.h>
#include <typeindex>
#include <ranges>
#include <algorithm>
#include <cassert>
// #include <value-ptr.hpp>


class IEntityField {
    protected:
        std::string m_field_name;
        std::uint32_t m_uid;

    public:
        IEntityField(std::string const& field_name, std::uint32_t const field_uid) : m_field_name(field_name),
            m_uid(field_uid) {
        };
        IEntityField(const IEntityField&) = default;
        IEntityField(IEntityField&&) noexcept = default;
        auto operator=(const IEntityField&) -> IEntityField& = default;
        auto operator=(IEntityField&&) -> IEntityField& = default;
        virtual ~IEntityField() = default;

        [[nodiscard]] virtual std::type_index get_type_index() const = 0;

        [[nodiscard]] std::uint32_t uid() const {
            return m_uid;
        }

        virtual bool changed(const IEntityField& other) const = 0;

        [[nodiscard]] std::string const& name() const { return m_field_name; }
        virtual IEntityField* clone() const = 0;
        virtual IEntityField* move_clone() const = 0;
};

constexpr bool some_of(auto&& r, auto&& pred) // some but not all
{
    return not(std::ranges::all_of(r, pred) or std::ranges::none_of(r, pred));
}

template <typename T>
class EntityField : public IEntityField, public std::optional<T> {
    public:
        EntityField(std::string const& field_name, std::uint32_t field_uid, T const& value)
            : IEntityField(field_name, field_uid), std::optional<T>(value) {
        }

        EntityField(std::string const& field_name, std::uint32_t field_uid, std::nullopt_t)
            : IEntityField(field_name, field_uid), std::optional<T>(std::nullopt) {
        }

        EntityField(std::string const& field_name, std::uint32_t field_uid, std::optional<T> value)
           : IEntityField(field_name, field_uid), std::optional<T>(value) {
        }

        EntityField(EntityField const& other)
            : IEntityField(other),
              std::optional<T>(other) {
        }

        EntityField(EntityField&& other) noexcept
            : IEntityField(std::move(other)),
              std::optional<T>(std::move(other)) {
        }

        EntityField& operator=(EntityField const& other) {
            if (this == &other)
                return *this;
            IEntityField::operator =(other);
            std::optional<T>::operator =(other);
            return *this;
        }

        EntityField& operator=(EntityField&& other) noexcept {
            if (this == &other)
                return *this;
            IEntityField::operator =(std::move(other));
            std::optional<T>::operator =(std::move(other));
            return *this;
        }

        IEntityField* clone() const override {
            return new EntityField(*this);
        }

        IEntityField* move_clone() const override {
            return new EntityField(std::move(*this));
        }

        [[nodiscard]] std::type_index get_type_index() const override {
            return std::type_index{typeid(std::remove_pointer_t<decltype(this)>)};
        }

        bool changed(IEntityField const& other) const override {
            assert(other.uid() == this->uid());
            assert(other.get_type_index() == this->get_type_index());
            try {
                EntityField<T> ofield = dynamic_cast<EntityField const&>(other);

                std::vector b{this->has_value(), ofield.has_value()};
                if (std::ranges::all_of(b, [](auto const& v) { return v; })) {
                    if (this->value() != ofield.value()) {
                        return true;
                    }
                    return false;
                }
                if (!std::ranges::none_of(b, [](auto const& v) { return v; })) {
                    return true;
                }
                return false;
            } catch (std::bad_cast const& e) {
                return false;
            }
        }

        using std::optional<T>::operator=;

        // using value_type = T;
        //
        // using std::optional<T>::optional;
        // using std::optional<T>::value;
        // using std::optional<T>::value_or;
        // using std::optional<T>::operator bool;
        // using std::optional<T>::operator*;
};


// template <>
// struct valuable::default_clone<IEntityField> {
//     default_clone() = default;
//     IEntityField *operator()(IEntityField const &x) const { return x.clone(); }
//     IEntityField *operator()(IEntityField &&x) const { return x.move_clone(); }
// };

class EntityFieldContainer {
    // On Destruction raw pointers will be cleaned up. like a garbage-collector
    // std::vector<valuable::value_ptr<IEntityField>> m_raii_fields;
    // Raw Pointers all for dynamic cast subobject casting
    protected:
        std::map<std::uint32_t, IEntityField*> m_fields;

    public:
        EntityFieldContainer() = default;

        EntityFieldContainer(EntityFieldContainer const& other) {
            for (auto const& [key, field] : other.m_fields) {
                m_fields[key] = field->clone();
            }
        }

        EntityFieldContainer(EntityFieldContainer const&& other) noexcept {
            for (auto const& [key, field] : other.m_fields) {
                m_fields[key] = field->move_clone();
            }
        }


        EntityFieldContainer& operator=(EntityFieldContainer const& other) {
            if (this == &other)
                return *this;
            for (auto const& [key, field] : other.m_fields) {
                m_fields[key] = field->clone();
            }
            return *this;
        }

        EntityFieldContainer& operator=(EntityFieldContainer&& other) noexcept {
            if (this == &other)
                return *this;
            for (auto const& [key, field] : other.m_fields) {
                m_fields[key] = field->move_clone();
            }
            return *this;
        }

        virtual ~EntityFieldContainer() {
            for (auto& [key, val] : m_fields) {
                delete val;
                val = nullptr;
            }
            m_fields.clear();
        }

        static EntityFieldContainer build(ldtk::EntityDef const& def) {
            EntityFieldContainer container;
            container.add_fields_from_entity_def(def);
            return container;
        }

        std::vector<uint32_t> enumerate_fields() const {
            return m_fields | std::views::keys | std::ranges::to<std::vector<uint32_t>>();
        }


        // Getting Fields

        IEntityField& get_field(std::string const& name) {
            for (auto& val : m_fields | std::views::values) {
                if (val->name() == name) {
                    return *val;
                }
            }
            throw std::runtime_error(std::format("Could not find field with name: '{}'", name));
        }

        IEntityField& get_field(std::uint32_t const id) {
            for (auto& [key,val] : m_fields) {
                if (key == id) {
                    return *val;
                }
            }
            throw std::runtime_error(std::format("Could not find field with name: '{}'", id));
        }

        template <typename T>
        EntityField<T>& get_field(std::string const& name) {
            for (auto val : m_fields | std::views::values) {
                if (val->name() == name) {
                    auto* casted_field = dynamic_cast<EntityField<T>*>(val);
                    if (casted_field) {
                        return *casted_field;
                    }
                    throw std::runtime_error(std::format("Could not convert field '{}' to type '{}'", name,
                                                         typeid(T).name()));
                }
            }
            throw std::runtime_error(std::format("Could not find field with name: '{}'", name));
        }

        // template <typename T>
        // EntityField<T>& get_field(std::string const& name) const {
        //     // FIXME: Use c++26 std::expected or some backported equivalent as exceptions prevent compiler optimisation.
        //     for (auto& val : m_fields | std::views::values) {
        //         if (val->name() == name) {
        //             auto* casted_field = dynamic_cast<EntityField<T>*>(val);
        //             if (casted_field) {
        //                 return *casted_field;
        //             }
        //             throw std::runtime_error(std::format("Could not convert field '{}' to type '{}'", name,
        //                                                  typeid(T).name()));
        //         }
        //     }
        //     throw std::runtime_error(std::format("Could not find field with name: '{}'", name));
        // }

        template <typename T>
        EntityField<T>& get_field(std::uint32_t uid) {
            for (auto& [key, value] : m_fields) {
                if (key == uid) {
                    auto* casted_field = dynamic_cast<EntityField<T>*>(value);
                    if (casted_field) {
                        return *casted_field;
                    }
                    throw std::runtime_error(std::format("Could not convert field '{}' ({}) to type '{}'",
                                                         value->name(), value->uid(),
                                                         typeid(T).name()));
                }
            }
            throw std::runtime_error(std::format("Could not find field with uid: '{}'", uid));
        }

        // template <typename T>
        // EntityField<T>& get_field(std::uint32_t const& uid) const {
        //     for (auto& [key, value] : m_fields) {
        //         if (key == uid) {
        //             auto* casted_field = dynamic_cast<EntityField<T>*>(value);
        //             if (casted_field) {
        //                 return *casted_field;
        //             }
        //             throw std::runtime_error(std::format("Could not convert field '{}' ({}) to type '{}'", value->name(), value->uid(),
        //                                                  typeid(T).name()));
        //         }
        //     }
        //     throw std::runtime_error(std::format("Could not find field with uid: '{}'", uid));
        // }

        // protected:
        // Adding Fields
        template <typename T>
        void add_field(std::string const& name, std::uint32_t uid, T const& field) {
            // auto* pfield = new ;
            // m_raii_fields.emplace_back(pfield);
            // m_fields[uid] = EntityField<int>(name, uid, field);
            m_fields[uid] = new EntityField<T>(name, uid, field);
        }

        // FIXME: Remove and instead use 2 parameter add_field
        template <typename T>
        void add_field(std::string const& name, std::uint32_t uid, std::nullopt_t) {
            // auto* pfield = new ;
            // m_raii_fields.emplace_back(pfield);
            m_fields[uid] = new EntityField<T>(name, uid, std::nullopt);
        }

        // FIXME: Remove and instead use 2 parameter add_field
        template <typename T>
        void add_field(std::string const& name, std::uint32_t uid, std::optional<T> value) {
            // auto* pfield = new ;
            // m_raii_fields.emplace_back(pfield);
            m_fields[uid] = new EntityField<T>(name, uid, value);
        }

        template <typename T>
        void add_field(std::string const& name, std::uint32_t uid) {
            // auto* pfield = new ;
            // m_raii_fields.emplace_back(pfield);
            m_fields[uid] = new EntityField<T>(name, uid, std::nullopt);
        }

    private:
        void add_fields_from_entity_def(ldtk::EntityDef const& def);
};
