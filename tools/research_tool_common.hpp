#pragma once

#include "pvc/research.hpp"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace pvc::tool {


struct StateFingerprint {
    std::uint64_t first = 0;
    std::uint64_t second = 0;

    friend constexpr bool operator==(const StateFingerprint&,
                                     const StateFingerprint&) = default;
};

struct StateFingerprintHash {
    std::size_t operator()(const StateFingerprint& value) const noexcept {
        const auto mixed = value.first
                         ^ std::rotl(value.second, 23)
                         ^ (value.first >> 17U);
        return std::hash<std::uint64_t>{}(mixed);
    }
};

inline StateFingerprint analysis_fingerprint(const InternalStateSnapshot& state) {
    StateFingerprint result{
        .first = 0x9e3779b97f4a7c15ULL,
        .second = 0xd1b54a32d192ed03ULL,
    };
    std::size_t index = 0;
    for (const auto byte : state.cube.storage()) {
        const auto term = static_cast<std::uint64_t>(byte)
                        + static_cast<std::uint64_t>(index + 1U);
        result.first = result.first * 257ULL + term;
        result.second = std::rotl(result.second + term * 131ULL, 11)
                      ^ result.first;
        ++index;
    }
    const std::array<std::uint8_t, 4> geometry{
        state.cursor.x, state.cursor.y, state.cursor.z,
        static_cast<std::uint8_t>(state.previous_axis)
    };
    for (const auto byte : geometry) {
        const auto term = static_cast<std::uint64_t>(byte)
                        + static_cast<std::uint64_t>(index + 1U);
        result.first = result.first * 257ULL + term;
        result.second = std::rotl(result.second + term * 131ULL, 11)
                      ^ result.first;
        ++index;
    }
    for (std::size_t i = 0; i < 8U; ++i) {
        const auto byte = static_cast<std::uint8_t>(state.symbol_index >> (i * 8U));
        const auto term = static_cast<std::uint64_t>(byte)
                        + static_cast<std::uint64_t>(index + 1U);
        result.first = result.first * 257ULL + term;
        result.second = std::rotl(result.second + term * 131ULL, 11)
                      ^ result.first;
        ++index;
    }
    return result;
}


inline std::uint64_t fast_state_fingerprint64(const InternalStateSnapshot& state) {
    // Research-only screening fingerprint. Exact state equality is always
    // checked before a collision is reported. Process eight bytes at a time
    // to keep exhaustive domain scans practical.
    std::uint64_t hash = 0x243f6a8885a308d3ULL;
    const auto& storage = state.cube.storage();
    for (std::size_t offset = 0; offset < storage.size(); offset += 8U) {
        std::uint64_t word = 0;
        std::memcpy(&word, storage.data() + offset, sizeof(word));
        word += 0x9e3779b97f4a7c15ULL
              + static_cast<std::uint64_t>(offset) * 0x100000001b3ULL;
        word ^= word >> 29U;
        word *= 0xbf58476d1ce4e5b9ULL;
        word ^= word >> 31U;
        hash ^= std::rotl(word, static_cast<int>((offset / 8U) & 63U));
        hash = std::rotl(hash, 17) * 0x94d049bb133111ebULL;
    }
    const auto geometry = static_cast<std::uint64_t>(state.cursor.x)
                        | (static_cast<std::uint64_t>(state.cursor.y) << 8U)
                        | (static_cast<std::uint64_t>(state.cursor.z) << 16U)
                        | (static_cast<std::uint64_t>(state.previous_axis) << 24U);
    hash ^= geometry * 0xd6e8feb86659fd93ULL;
    hash ^= std::rotl(state.symbol_index * 0xa0761d6478bd642fULL, 23);
    hash ^= hash >> 32U;
    hash *= 0xe7037ed1a0b428dbULL;
    hash ^= hash >> 29U;
    return hash;
}

struct StateKeyLess {
    bool operator()(const std::vector<std::uint8_t>& left,
                    const std::vector<std::uint8_t>& right) const noexcept {
        const auto common = left.size() < right.size() ? left.size() : right.size();
        for (std::size_t i = 0; i < common; ++i) {
            if (left[i] < right[i]) {
                return true;
            }
            if (left[i] > right[i]) {
                return false;
            }
        }
        return left.size() < right.size();
    }
};

inline std::vector<std::uint8_t> serialize_state(const InternalStateSnapshot& state) {
    std::vector<std::uint8_t> bytes;
    bytes.reserve(kCubeCells + 12U);
    const auto& storage = state.cube.storage();
    bytes.insert(bytes.end(), storage.begin(), storage.end());
    bytes.push_back(state.cursor.x);
    bytes.push_back(state.cursor.y);
    bytes.push_back(state.cursor.z);
    bytes.push_back(static_cast<std::uint8_t>(state.previous_axis));
    for (std::size_t i = 0; i < 8U; ++i) {
        bytes.push_back(static_cast<std::uint8_t>(state.symbol_index >> (i * 8U)));
    }
    return bytes;
}

inline std::vector<std::uint8_t> counter_message(std::uint64_t counter,
                                                 std::uint64_t domain,
                                                 std::size_t size) {
    if (size < 16U) {
        throw std::invalid_argument("counter_message requires at least 16 bytes");
    }
    std::vector<std::uint8_t> message(size);
    for (std::size_t i = 0; i < 8U; ++i) {
        message[i] = static_cast<std::uint8_t>(counter >> (i * 8U));
        message[8U + i] = static_cast<std::uint8_t>(domain >> (i * 8U));
    }
    for (std::size_t i = 16U; i < size; ++i) {
        const auto a = static_cast<std::uint8_t>(counter >> ((i & 7U) * 8U));
        const auto b = static_cast<std::uint8_t>(domain >> (((i + 3U) & 7U) * 8U));
        const auto left = static_cast<std::uint8_t>(
            a + static_cast<std::uint8_t>(i * 37U));
        const auto right = static_cast<std::uint8_t>(
            b + static_cast<std::uint8_t>(i * 11U));
        message[i] = static_cast<std::uint8_t>(left ^ right);
    }
    return message;
}

inline std::string hex_bytes(const std::vector<std::uint8_t>& bytes) {
    static constexpr char digits[] = "0123456789abcdef";
    std::string out;
    out.reserve(bytes.size() * 2U);
    for (const auto byte : bytes) {
        out.push_back(digits[(byte >> 4U) & 0x0FU]);
        out.push_back(digits[byte & 0x0FU]);
    }
    return out;
}

inline std::string sequence_hex(std::uint32_t sequence, std::size_t depth) {
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < depth; ++i) {
        const auto shift = static_cast<unsigned>((depth - 1U - i) * 8U);
        out << std::setw(2) << ((sequence >> shift) & 0xFFU);
        if (i + 1U != depth) {
            out << ' ';
        }
    }
    return out.str();
}

inline std::size_t byte_hamming_distance(std::span<const std::uint8_t> left,
                                         std::span<const std::uint8_t> right) {
    if (left.size() != right.size()) {
        throw std::invalid_argument("Hamming distance requires equal lengths");
    }
    std::size_t distance = 0;
    for (std::size_t i = 0; i < left.size(); ++i) {
        distance += static_cast<std::size_t>(
            std::popcount(static_cast<unsigned>(left[i] ^ right[i])));
    }
    return distance;
}

inline std::size_t differing_bytes(std::span<const std::uint8_t> left,
                                   std::span<const std::uint8_t> right) {
    if (left.size() != right.size()) {
        throw std::invalid_argument("byte distance requires equal lengths");
    }
    std::size_t distance = 0;
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (left[i] != right[i]) {
            ++distance;
        }
    }
    return distance;
}

inline std::size_t cube_bit_distance(const Cube& left, const Cube& right) {
    return byte_hamming_distance(left.storage(), right.storage());
}

inline std::size_t cube_byte_distance(const Cube& left, const Cube& right) {
    return differing_bytes(left.storage(), right.storage());
}

inline const NamedHashParameters& find_preset(std::string_view name) {
    static const auto presets = reduced_round_presets();
    for (const auto& preset : presets) {
        if (preset.name == name) {
            return preset;
        }
    }
    throw std::invalid_argument("unknown reduced-round preset");
}

inline void print_parameters(const HashParameters& parameters) {
    std::cout << "moves_per_symbol=" << parameters.moves_per_symbol
              << " diagonal_closure=" << parameters.diagonal_closure_symbols
              << " orbit_closure=" << parameters.orbit_closure_symbols
              << " squeeze_bytes=" << parameters.squeeze_bytes
              << " squeeze_symbols_per_byte=" << parameters.squeeze_symbols_per_byte
              << " foldback=" << (parameters.enable_foldback ? "on" : "off")
              << '\n';
}

} // namespace pvc::tool
