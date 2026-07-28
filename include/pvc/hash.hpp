#pragma once

#include "pvc/cube.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace pvc {

inline constexpr std::size_t kDigestBytes = 32;
inline constexpr std::size_t kMovesPerSymbol = 6;
inline constexpr std::size_t kClosureSymbols = 32;

using Digest = std::array<std::uint8_t, kDigestBytes>;

struct Move {
    Axis axis{};
    Coord intersection_before{};
    Coord intersection_after{};
    std::uint8_t amount{};
    std::uint8_t symbol{};
    std::uint8_t phase{};
    std::uint64_t symbol_index{};
};

struct HashResult {
    Digest digest{};
    Cube final_cube{};
    std::vector<Move> trace{};
    std::uint64_t input_size{};
};

class RotHash0 {
public:
    RotHash0();

    void update(std::span<const std::uint8_t> bytes);
    void update(std::string_view text);

    [[nodiscard]] HashResult finalize(bool keep_trace = false) const;

    [[nodiscard]] static Digest hash(std::span<const std::uint8_t> bytes);
    [[nodiscard]] static Digest hash(std::string_view text);
    [[nodiscard]] static HashResult inspect(std::span<const std::uint8_t> bytes,
                                            bool keep_trace = true);

private:
    std::vector<std::uint8_t> message_{};
};

[[nodiscard]] std::size_t digest_hamming_distance(const Digest& left,
                                                  const Digest& right);
[[nodiscard]] std::size_t digest_byte_distance(const Digest& left,
                                               const Digest& right);

} // namespace pvc
