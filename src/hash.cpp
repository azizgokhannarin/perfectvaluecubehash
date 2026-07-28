#include "pvc/hash.hpp"

#include <bit>
#include <limits>
#include <stdexcept>

namespace pvc {
namespace {

struct WorkingState {
    Cube cube = Cube::perfect();
    Coord cursor{0, 0, 0};
    Axis previous_axis = Axis::X;
    std::uint64_t symbol_index = 0;
    std::vector<Move>* trace = nullptr;
};

constexpr std::uint8_t rotate_left_byte(std::uint8_t value, unsigned amount) {
    const unsigned shift = amount & 7U;
    if (shift == 0U) {
        return value;
    }
    const auto wide = static_cast<unsigned>(value);
    return static_cast<std::uint8_t>(((wide << shift) | (wide >> (8U - shift))) & 0xFFU);
}

constexpr std::uint8_t coordinate_code(Coord coord) {
    return static_cast<std::uint8_t>(
        (static_cast<unsigned>(coord.x) << 5U)
        ^ (static_cast<unsigned>(coord.y) << 2U)
        ^ static_cast<unsigned>(coord.z));
}

Axis choose_other_axis(Axis previous, std::uint8_t selector) {
    const bool second = (selector & 1U) != 0U;
    switch (previous) {
    case Axis::X: return second ? Axis::Z : Axis::Y;
    case Axis::Y: return second ? Axis::X : Axis::Z;
    case Axis::Z: return second ? Axis::Y : Axis::X;
    }
    return Axis::Y;
}

std::uint8_t axis_code(Axis axis) {
    return static_cast<std::uint8_t>(axis);
}

void absorb_symbol(WorkingState& state, std::uint8_t symbol) {
    std::uint8_t control = static_cast<std::uint8_t>(
        symbol
        + static_cast<std::uint8_t>(state.symbol_index)
        + coordinate_code(state.cursor));

    for (std::uint8_t phase = 0; phase < kMovesPerSymbol; ++phase) {
        const auto intersection_before = state.cursor;
        const auto probe_before = state.cube.at(state.cursor);
        const auto index_byte = static_cast<std::uint8_t>(
            state.symbol_index >> static_cast<unsigned>((phase & 7U) * 8U));
        const auto geometry = static_cast<std::uint8_t>(
            coordinate_code(state.cursor)
            + static_cast<std::uint8_t>(phase * 29U)
            + index_byte);

        const auto selector = static_cast<std::uint8_t>(
            rotate_left_byte(control, phase)
            ^ probe_before
            ^ geometry);

        const Axis axis = choose_other_axis(state.previous_axis, selector);

        const auto amount_source = static_cast<unsigned>(control)
                                 + static_cast<unsigned>(probe_before)
                                 + static_cast<unsigned>(geometry)
                                 + static_cast<unsigned>(symbol)
                                 + static_cast<unsigned>(axis_code(axis) * 11U);

        const auto amount =
            static_cast<std::uint8_t>(1U + (amount_source % 7U));

        state.cube.rotate_line(axis, state.cursor, amount);
        state.cursor = advance(state.cursor, axis, amount);

        const auto probe_after = state.cube.at(state.cursor);

        if (state.trace != nullptr) {
            state.trace->push_back(Move{
                .axis = axis,
                .intersection_before = intersection_before,
                .intersection_after = state.cursor,
                .amount = amount,
                .symbol = symbol,
                .phase = phase,
                .symbol_index = state.symbol_index,
            });
        }

        control = static_cast<std::uint8_t>(
            rotate_left_byte(control, 1U + (axis_code(axis) & 1U))
            + probe_after
            + amount
            + coordinate_code(state.cursor)
            + static_cast<std::uint8_t>(phase * 7U));

        state.previous_axis = axis;
    }

    ++state.symbol_index;
}

std::array<std::uint8_t, 8> encode_length(std::uint64_t size) {
    std::array<std::uint8_t, 8> bytes{};
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<std::uint8_t>(size >> (i * 8U));
    }
    return bytes;
}

void absorb_closure(WorkingState& state, std::uint64_t input_size) {
    // Geometry-derived frame markers:
    // 252 = 1020 mod 256, 8 = cube side.
    absorb_symbol(state, 252U);

    const auto length = encode_length(input_size);
    const auto perfect_diagonals = Cube::perfect().body_diagonals();

    for (std::size_t i = 0; i < length.size(); ++i) {
        const auto framed = static_cast<std::uint8_t>(
            length[i]
            + perfect_diagonals[i * 4U]
            + static_cast<std::uint8_t>(i * 8U));
        absorb_symbol(state, framed);
    }

    absorb_symbol(state, 8U);

    // Self-fed closure: every step samples the current body diagonals, so
    // subsequent closure moves depend on earlier closure moves.
    for (std::size_t i = 0; i < kClosureSymbols; ++i) {
        const auto current = state.cube.body_diagonals();
        const auto length_part = length[i & 7U];
        const auto symbol = static_cast<std::uint8_t>(
            current[i]
            ^ current[(i + 11U) & 31U]
            ^ perfect_diagonals[(i * 7U) & 31U]
            ^ length_part
            ^ static_cast<std::uint8_t>(i * 8U + (i >> 2U)));
        absorb_symbol(state, symbol);
    }
}

HashResult compute(std::span<const std::uint8_t> bytes, bool keep_trace) {
    WorkingState state;
    std::vector<Move> trace;
    if (keep_trace) {
        trace.reserve((bytes.size() + 42U) * kMovesPerSymbol);
        state.trace = &trace;
    }

    for (const auto byte : bytes) {
        absorb_symbol(state, byte);
    }

    // Foldback pass: the same message is traversed from the far end back
    // toward the start.  The canonical diagonal and original byte position
    // change the return symbol.  This binds the continuation to the exact
    // path, not only to a possibly convergent forward cube state.
    const auto canonical_diagonals = Cube::perfect().body_diagonals();
    for (std::size_t reverse = bytes.size(); reverse > 0U; --reverse) {
        const std::size_t original_index = reverse - 1U;
        const auto return_symbol = static_cast<std::uint8_t>(
            bytes[original_index]
            ^ canonical_diagonals[original_index & 31U]
            ^ static_cast<std::uint8_t>(original_index * 8U + (original_index >> 3U)));
        absorb_symbol(state, return_symbol);
    }

    absorb_closure(state, static_cast<std::uint64_t>(bytes.size()));

    HashResult result;
    result.digest = state.cube.body_diagonals();
    result.final_cube = state.cube;
    result.input_size = static_cast<std::uint64_t>(bytes.size());
    if (keep_trace) {
        result.trace = std::move(trace);
    }
    return result;
}

} // namespace

RotHash0::RotHash0() = default;

void RotHash0::update(std::span<const std::uint8_t> bytes) {
    if (bytes.size() > std::numeric_limits<std::size_t>::max() - message_.size()) {
        throw std::length_error("PVC-RotHash-0 message is too large");
    }
    message_.insert(message_.end(), bytes.begin(), bytes.end());
}

void RotHash0::update(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    update(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash0::finalize(bool keep_trace) const {
    return compute(message_, keep_trace);
}

Digest RotHash0::hash(std::span<const std::uint8_t> bytes) {
    return compute(bytes, false).digest;
}

Digest RotHash0::hash(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    return hash(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash0::inspect(std::span<const std::uint8_t> bytes, bool keep_trace) {
    return compute(bytes, keep_trace);
}

std::size_t digest_hamming_distance(const Digest& left, const Digest& right) {
    std::size_t distance = 0;
    for (std::size_t i = 0; i < left.size(); ++i) {
        distance += static_cast<std::size_t>(
            std::popcount(static_cast<unsigned>(left[i] ^ right[i])));
    }
    return distance;
}

std::size_t digest_byte_distance(const Digest& left, const Digest& right) {
    std::size_t distance = 0;
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (left[i] != right[i]) {
            ++distance;
        }
    }
    return distance;
}

} // namespace pvc
