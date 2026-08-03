#pragma once
#include <type_traits>



template <typename FlagBitsType>
struct FlagTraits {
    static constexpr bool isBitmask = false;
};

template <typename BitType>
class Flags {
    public:
        using BitsType = BitType;
        using MaskType = std::underlying_type_t<BitType>;

        // constructors
        constexpr Flags() noexcept : m_mask(0) {
        }

        constexpr Flags(BitType bit) noexcept : m_mask(static_cast<MaskType>(bit)) {
        }

        constexpr Flags(Flags<BitType> const& rhs) noexcept = default;

        constexpr explicit Flags(MaskType flags) noexcept : m_mask(flags) {
        }

        // relational operators
        // auto operator<=>(Flags<BitType> const&) const = default;

        constexpr bool operator<( Flags<BitType> const & rhs ) const noexcept
    {
        return m_mask < rhs.m_mask;
    }

        constexpr bool operator<=( Flags<BitType> const & rhs ) const noexcept
        {
            return m_mask <= rhs.m_mask;
        }

        constexpr bool operator>( Flags<BitType> const & rhs ) const noexcept
        {
            return m_mask > rhs.m_mask;
        }

        constexpr bool operator>=( Flags<BitType> const & rhs ) const noexcept
        {
            return m_mask >= rhs.m_mask;
        }

        constexpr bool operator==( Flags<BitType> const & rhs ) const noexcept
        {
            return m_mask == rhs.m_mask;
        }

        constexpr bool operator!=( Flags<BitType> const & rhs ) const noexcept
        {
            return m_mask != rhs.m_mask;
        }

        // logical operator
        constexpr bool operator!() const noexcept {
            return !m_mask;
        }

        // bitwise operators
        constexpr Flags<BitType> operator&(Flags<BitType> const& rhs) const noexcept {
            return Flags<BitType>(m_mask & rhs.m_mask);
        }

        constexpr Flags<BitType> operator|(Flags<BitType> const& rhs) const noexcept {
            return Flags<BitType>(m_mask | rhs.m_mask);
        }

        constexpr Flags<BitType> operator^(Flags<BitType> const& rhs) const noexcept {
            return Flags<BitType>(m_mask ^ rhs.m_mask);
        }

        constexpr Flags<BitType> operator~() const noexcept {
            return Flags<BitType>(m_mask ^ FlagTraits<BitType>::allFlags.m_mask);
        }

        // assignment operators
        constexpr Flags<BitType>& operator=(Flags<BitType> const& rhs) noexcept = default;

        constexpr Flags<BitType>& operator|=(Flags<BitType> const& rhs) noexcept {
            m_mask |= rhs.m_mask;
            return *this;
        }

        constexpr Flags<BitType>& operator&=(Flags<BitType> const& rhs) noexcept {
            m_mask &= rhs.m_mask;
            return *this;
        }

        constexpr Flags<BitType>& operator^=(Flags<BitType> const& rhs) noexcept {
            m_mask ^= rhs.m_mask;
            return *this;
        }

        // cast operators
        explicit constexpr operator bool() const noexcept {
            return !!m_mask;
        }

        explicit constexpr operator MaskType() const noexcept {
            return m_mask;
        }

        #if defined( VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC )
    public:
        #else

    private:
        #endif
        MaskType m_mask;
};


template <typename T>
concept Enum = std::is_enum_v<T>;

template <Enum T>
constexpr bool has_flag(T e, T flag) {
    return (e & flag) == flag;
}

template <Enum T>
constexpr bool has_flag(Flags<T> e, T flag) {
    return (e & flag) == flag;
}
