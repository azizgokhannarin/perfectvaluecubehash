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

constexpr Coord storage_coord(std::size_t index) {
    return Coord{
        static_cast<std::uint8_t>(index & 7U),
        static_cast<std::uint8_t>((index >> 3U) & 7U),
        static_cast<std::uint8_t>((index >> 6U) & 7U),
    };
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

    // A longer self-fed diagonal closure is intentionally used in v0.2.0.
    // Short inputs otherwise retain a measurable memory of the canonical
    // value originally stored at each output coordinate.
    for (std::size_t i = 0; i < kClosureSymbols; ++i) {
        const auto current = state.cube.body_diagonals();
        const auto length_part = length[i & 7U];
        const auto first = current[i & 31U];
        const auto second = current[(i + 11U) & 31U];
        const auto third = current[(i * 7U + 3U) & 31U];
        const auto folded = static_cast<std::uint8_t>(
            rotate_left_byte(first, 1U)
            + rotate_left_byte(second, 3U)
            + rotate_left_byte(third, 5U));
        const auto symbol = static_cast<std::uint8_t>(
            folded
            ^ perfect_diagonals[(i * 13U) & 31U]
            ^ length_part
            ^ static_cast<std::uint8_t>(i * 8U + (i >> 2U)));
        absorb_symbol(state, symbol);
    }
}

void absorb_orbit_closure(WorkingState& state, std::uint64_t input_size) {
    const auto length = encode_length(input_size);

    // 128 steps sample four quarters of the 512-cell cube. Across the full
    // orbit every physical cell participates once as a direct sample. Since
    // the cube changes after every step, later samples also observe the path
    // created by earlier samples.
    for (std::size_t i = 0; i < kOrbitSymbols; ++i) {
        const auto p0 = storage_coord(i);
        const auto p1 = storage_coord(i + 128U);
        const auto p2 = storage_coord(i + 256U);
        const auto p3 = storage_coord(i + 384U);

        const auto v0 = state.cube.at(p0);
        const auto v1 = state.cube.at(p1);
        const auto v2 = state.cube.at(p2);
        const auto v3 = state.cube.at(p3);
        const auto diagonals = state.cube.body_diagonals();

        const auto pair_a = static_cast<std::uint8_t>(
            rotate_left_byte(v0, 1U) + rotate_left_byte(v1, 3U));
        const auto pair_b = static_cast<std::uint8_t>(
            rotate_left_byte(v2, 5U) + rotate_left_byte(v3, 7U));
        const auto symbol = static_cast<std::uint8_t>(
            pair_a
            ^ pair_b
            ^ diagonals[(i * 5U + (i >> 3U)) & 31U]
            ^ length[i & 7U]
            ^ coordinate_code(state.cursor)
            ^ static_cast<std::uint8_t>(i * 4U + (i >> 5U)));

        absorb_symbol(state, symbol);
    }
}

std::uint8_t derive_diagonal_byte(const Cube::DigestCells& diagonals,
                                  std::size_t output_index,
                                  std::uint8_t chain) {
    const std::size_t lane = output_index >> 3U;
    const std::size_t position = output_index & 7U;

    const auto a = diagonals[((lane + 0U) & 3U) * 8U + position];
    const auto b = diagonals[((lane + 1U) & 3U) * 8U
                           + ((position + 1U + lane) & 7U)];
    const auto c = diagonals[((lane + 2U) & 3U) * 8U
                           + ((position + 3U + 2U * lane) & 7U)];
    const auto d = diagonals[((lane + 3U) & 3U) * 8U
                           + ((position + 5U + 3U * lane) & 7U)];

    const auto pair_a = static_cast<std::uint8_t>(
        rotate_left_byte(a, 1U) + rotate_left_byte(b, 3U));
    const auto pair_b = static_cast<std::uint8_t>(
        rotate_left_byte(c, 5U) + rotate_left_byte(d, 7U));
    const auto cross = static_cast<std::uint8_t>(
        diagonals[(output_index * 7U + 3U) & 31U]
        + diagonals[(output_index * 13U + 1U) & 31U]);

    return static_cast<std::uint8_t>(
        rotate_left_byte(static_cast<std::uint8_t>(pair_a ^ pair_b ^ chain),
                         1U + static_cast<unsigned>(position))
        + rotate_left_byte(cross, 1U + static_cast<unsigned>(lane * 2U))
        + static_cast<std::uint8_t>(output_index * 8U + lane));
}

Digest squeeze_diagonals(WorkingState& state, std::uint64_t input_size) {
    Digest digest{};
    const auto length = encode_length(input_size);
    auto initial = state.cube.body_diagonals();
    std::uint8_t chain = static_cast<std::uint8_t>(
        initial[0]
        ^ rotate_left_byte(initial[9], 1U)
        ^ rotate_left_byte(initial[18], 3U)
        ^ rotate_left_byte(initial[27], 5U)
        ^ length[0]);

    for (std::size_t i = 0; i < digest.size(); ++i) {
        const auto diagonals = state.cube.body_diagonals();
        const auto output = derive_diagonal_byte(diagonals, i, chain);
        digest[i] = output;

        // Four state-dependent symbols, one rooted in each body diagonal,
        // separate consecutive output bytes into distinct cube states.
        for (std::size_t diagonal = 0; diagonal < kSqueezeSymbolsPerByte; ++diagonal) {
            const auto current = state.cube.body_diagonals();
            const std::size_t position = i & 7U;
            const std::size_t lane = i >> 3U;
            const auto left = current[diagonal * 8U
                                    + ((position + diagonal + lane) & 7U)];
            const auto right = current[((diagonal + 1U) & 3U) * 8U
                                     + ((7U - position + lane + diagonal) & 7U)];
            const auto symbol = static_cast<std::uint8_t>(
                rotate_left_byte(left, 1U + static_cast<unsigned>(2U * diagonal))
                + rotate_left_byte(right, 7U - static_cast<unsigned>(2U * diagonal))
                + rotate_left_byte(output, 1U + static_cast<unsigned>(diagonal))
                + chain
                + length[(i + diagonal) & 7U]
                + static_cast<std::uint8_t>(i * 4U + diagonal));
            absorb_symbol(state, symbol);
        }

        const auto after = state.cube.body_diagonals();
        chain = static_cast<std::uint8_t>(
            rotate_left_byte(chain, 1U + static_cast<unsigned>(i & 7U))
            + output
            + after[(i * 5U + 7U) & 31U]
            + state.cube.at(state.cursor)
            + coordinate_code(state.cursor));
    }

    return digest;
}

HashResult compute(std::span<const std::uint8_t> bytes, bool keep_trace) {
    WorkingState state;
    std::vector<Move> trace;
    if (keep_trace) {
        const auto symbol_count = bytes.size() * 2U
                                + 2U + 8U
                                + kClosureSymbols
                                + kOrbitSymbols
                                + kDigestBytes * kSqueezeSymbolsPerByte;
        trace.reserve(symbol_count * kMovesPerSymbol);
        state.trace = &trace;
    }

    for (const auto byte : bytes) {
        absorb_symbol(state, byte);
    }

    // Foldback pass binds the final state to the ordered message path rather
    // than only to a possibly convergent forward state.
    const auto canonical_diagonals = Cube::perfect().body_diagonals();
    for (std::size_t reverse = bytes.size(); reverse > 0U; --reverse) {
        const std::size_t original_index = reverse - 1U;
        const auto return_symbol = static_cast<std::uint8_t>(
            bytes[original_index]
            ^ canonical_diagonals[original_index & 31U]
            ^ static_cast<std::uint8_t>(original_index * 8U + (original_index >> 3U)));
        absorb_symbol(state, return_symbol);
    }

    const auto input_size = static_cast<std::uint64_t>(bytes.size());
    absorb_closure(state, input_size);
    absorb_orbit_closure(state, input_size);

    HashResult result;
    result.digest = squeeze_diagonals(state, input_size);
    result.final_cube = state.cube;
    result.input_size = input_size;
    if (keep_trace) {
        result.trace = std::move(trace);
    }
    return result;
}

} // namespace

RotHash1::RotHash1() = default;

void RotHash1::update(std::span<const std::uint8_t> bytes) {
    if (bytes.size() > std::numeric_limits<std::size_t>::max() - message_.size()) {
        throw std::length_error("PVC-RotHash-1 message is too large");
    }
    message_.insert(message_.end(), bytes.begin(), bytes.end());
}

void RotHash1::update(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    update(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash1::finalize(bool keep_trace) const {
    return compute(message_, keep_trace);
}

Digest RotHash1::hash(std::span<const std::uint8_t> bytes) {
    return compute(bytes, false).digest;
}

Digest RotHash1::hash(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    return hash(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash1::inspect(std::span<const std::uint8_t> bytes, bool keep_trace) {
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
