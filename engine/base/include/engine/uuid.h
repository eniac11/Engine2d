#pragma once
#include <array>
#include <atomic>
#include <format>
#include <random>
#include <string>
#include <ostream>
#include <iterator>
#include <chrono>
#include "engine/resources/sha1.h"



using _uuid = std::array<std::uint8_t, 16>;

struct  uuid : _uuid {

};

template<>
struct std::formatter<uuid> {
    template<class ParseContext>
    constexpr typename ParseContext::iterator parse(ParseContext& context) {
        return context.begin();
    }

    template <class FormatContext>
    typename FormatContext::iterator format(uuid value, FormatContext& context) const {
        return std::format_to(context.out(), "{:n}", static_cast<_uuid>(value));
    }
};


inline std::ostream& operator<<(std::ostream& strm, uuid const& value) {
    std::ranges::copy(static_cast<_uuid>(value), std::ostream_iterator<_uuid::value_type>(strm, " "));
    return strm;
}



namespace detail {
    constexpr uuid uuid5_impl(std::array<std::uint8_t, 16 + 256> buffer, std::size_t len) {

        auto hash = sha1(buffer.data(), len);

        uuid out{};
        for (int i = 0; i < 16; ++i)
            out[i] = hash.at(i);

        // Version = 5
        out[6] = (out[6] & std::uint8_t(0x0F)) | std::uint8_t(0x50);

        // Variant = RFC 4122
        out[8] = (out[8] & std::uint8_t(0x3F)) | std::uint8_t(0x80);

        return out;

    }
}

constexpr uuid uuid5(const uuid& ns, const std::string& name)
{
    std::array<std::uint8_t, 16 + 256> buffer{};
    std::size_t len = 0;

    for (std::uint8_t const b : ns)
        buffer[len++] = b;

    for (char const c : name)
        buffer[len++] = std::uint8_t(c);

    return detail::uuid5_impl(buffer, len);
}

constexpr uuid uuid5(const uuid& ns, const uuid& name)
{
    std::array<std::uint8_t, 16 + 256> buffer{};
    std::size_t len = 0;

    for (std::uint8_t const b : ns)
        buffer[len++] = b;

    for (std::uint8_t const c : name)
        buffer[len++] = c;

    return detail::uuid5_impl(buffer, len);
}

consteval uuid uuid5_ct(const uuid& ns, const std::string& name) {
    return uuid5(ns, name);
}

consteval uuid uuid5_ct(const uuid& ns, const uuid& name) {
    return uuid5(ns, name);
}

// Global monotonic state (per-process)
inline std::atomic<std::uint64_t> last_ms{0};
inline std::atomic<std::uint16_t> sequence{0};

inline std::uint64_t unix_epoch_ms()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(
        system_clock::now().time_since_epoch()
    ).count();
}

inline uuid uuid7()
{
    uuid out{};

    // ---- timestamp + sequence ----
    std::uint64_t ms = unix_epoch_ms();
    std::uint16_t seq;

    std::uint64_t prev = last_ms.load(std::memory_order_relaxed);
    if (ms == prev)
    {
        seq = sequence.fetch_add(1, std::memory_order_relaxed) & 0x0FFF;
    }
    else
    {
        last_ms.store(ms, std::memory_order_relaxed);
        sequence.store(0, std::memory_order_relaxed);
        seq = 0;
    }

    // ---- randomness ----
    static thread_local std::mt19937_64 rng{ std::random_device{}() };
    std::uint64_t rand62 = rng() & ((1ULL << 62) - 1);

    // ---- pack bytes (big endian) ----
    out[0] = std::uint8_t(ms >> 40);
    out[1] = std::uint8_t(ms >> 32);
    out[2] = std::uint8_t(ms >> 24);
    out[3] = std::uint8_t(ms >> 16);
    out[4] = std::uint8_t(ms >> 8);
    out[5] = std::uint8_t(ms);

    out[6] = std::uint8_t(0x70 | ((seq >> 8) & 0x0F)); // version 7
    out[7] = std::uint8_t(seq & 0xFF);

    out[8] = std::uint8_t(0x80 | (rand62 >> 56));     // variant RFC 4122
    out[9] = std::uint8_t(rand62 >> 48);
    out[10] = std::uint8_t(rand62 >> 40);
    out[11] = std::uint8_t(rand62 >> 32);
    out[12] = std::uint8_t(rand62 >> 24);
    out[13] = std::uint8_t(rand62 >> 16);
    out[14] = std::uint8_t(rand62 >> 8);
    out[15] = std::uint8_t(rand62);

    return out;
}