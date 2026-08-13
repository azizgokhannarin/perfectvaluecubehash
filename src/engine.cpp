#include "engine.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>

namespace pvc::detail {
namespace {

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

std::array<std::uint8_t, 8> encode_length(std::uint64_t size) {
    std::array<std::uint8_t, 8> bytes{};
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<std::uint8_t>(size >> (i * 8U));
    }
    return bytes;
}

void add_checkpoint(std::vector<StateCheckpoint>* checkpoints,
                    ResearchPhase phase,
                    std::size_t index,
                    const WorkingState& state) {
    if (checkpoints != nullptr) {
        checkpoints->push_back(StateCheckpoint{
            .phase = phase,
            .index = index,
            .state = snapshot(state),
        });
    }
}

void absorb_closure(WorkingState& state,
                    std::uint64_t input_size,
                    const HashParameters& parameters) {
    // Geometry-derived frame markers:
    // 252 = 1020 mod 256, 8 = cube side.
    absorb_symbol(state, 252U, parameters);

    const auto length = encode_length(input_size);
    const auto perfect_diagonals = Cube::perfect().body_diagonals();

    for (std::size_t i = 0; i < length.size(); ++i) {
        const auto framed = static_cast<std::uint8_t>(
            length[i]
            + perfect_diagonals[i * 4U]
            + static_cast<std::uint8_t>(i * 8U));
        absorb_symbol(state, framed, parameters);
    }

    absorb_symbol(state, 8U, parameters);

    for (std::size_t i = 0; i < parameters.diagonal_closure_symbols; ++i) {
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
        absorb_symbol(state, symbol, parameters);
    }
}

void absorb_orbit_closure(WorkingState& state,
                          std::uint64_t input_size,
                          const HashParameters& parameters) {
    const auto length = encode_length(input_size);

    for (std::size_t i = 0; i < parameters.orbit_closure_symbols; ++i) {
        // The canonical 128-step orbit covers each quarter exactly once.
        // Reduced and extended experiments wrap modulo 128 while retaining
        // the same four-quarter geometry.
        const std::size_t offset = i & 127U;
        const auto p0 = storage_coord(offset);
        const auto p1 = storage_coord(offset + 128U);
        const auto p2 = storage_coord(offset + 256U);
        const auto p3 = storage_coord(offset + 384U);

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

        absorb_symbol(state, symbol, parameters);
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

std::vector<std::uint8_t> squeeze_diagonals(
    WorkingState& state,
    std::uint64_t input_size,
    const HashParameters& parameters,
    std::vector<StateCheckpoint>* checkpoints) {
    std::vector<std::uint8_t> digest(parameters.squeeze_bytes);
    const auto length = encode_length(input_size);
    const auto initial = state.cube.body_diagonals();
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

        for (std::size_t diagonal = 0;
             diagonal < parameters.squeeze_symbols_per_byte;
             ++diagonal) {
            const auto current = state.cube.body_diagonals();
            const std::size_t position = i & 7U;
            const std::size_t lane = i >> 3U;
            const std::size_t diagonal_lane = diagonal & 3U;
            const auto left = current[diagonal_lane * 8U
                                    + ((position + diagonal + lane) & 7U)];
            const auto right = current[((diagonal_lane + 1U) & 3U) * 8U
                                     + ((7U - position + lane + diagonal) & 7U)];
            const auto symbol = static_cast<std::uint8_t>(
                rotate_left_byte(left, 1U + static_cast<unsigned>(2U * diagonal_lane))
                + rotate_left_byte(right, 7U - static_cast<unsigned>(2U * diagonal_lane))
                + rotate_left_byte(output, 1U + static_cast<unsigned>(diagonal & 7U))
                + chain
                + length[(i + diagonal) & 7U]
                + static_cast<std::uint8_t>(i * parameters.squeeze_symbols_per_byte
                                          + diagonal));
            absorb_symbol(state, symbol, parameters);
        }

        const auto after = state.cube.body_diagonals();
        chain = static_cast<std::uint8_t>(
            rotate_left_byte(chain, 1U + static_cast<unsigned>(i & 7U))
            + output
            + after[(i * 5U + 7U) & 31U]
            + state.cube.at(state.cursor)
            + coordinate_code(state.cursor));

        add_checkpoint(checkpoints, ResearchPhase::AfterSqueezeByte, i, state);
    }

    return digest;
}

} // namespace

InternalStateSnapshot snapshot(const WorkingState& state) {
    return InternalStateSnapshot{
        .cube = state.cube,
        .cursor = state.cursor,
        .previous_axis = state.previous_axis,
        .symbol_index = state.symbol_index,
    };
}

WorkingState working_state_from(const InternalStateSnapshot& state,
                                std::vector<Move>* trace) {
    return WorkingState{
        .cube = state.cube,
        .cursor = state.cursor,
        .previous_axis = state.previous_axis,
        .symbol_index = state.symbol_index,
        .trace = trace,
    };
}

constexpr std::uint8_t mul_odd_byte(std::uint8_t value, std::uint8_t odd) {
    return static_cast<std::uint8_t>(
        (static_cast<unsigned>(value) * static_cast<unsigned>(odd)) & 0xFFU);
}

// PVC-RotHash-2 / Controller S: systematic injectivity channel (phases 0–2)
// + free diffusion (phases 3–5). See docs/EXTERNAL_ADVICE_G3.md.
void absorb_symbol_systematic(WorkingState& state, std::uint8_t symbol) {
    const auto probe0 = state.cube.at(state.cursor);
    const auto context_seed = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(state.symbol_index)
        ^ coordinate_code(state.cursor)
        ^ static_cast<std::uint8_t>(axis_code(state.previous_axis) * 13U)
        ^ mul_odd_byte(probe0, 19U));

    std::array<std::uint8_t, 3> c_digits{};
    for (std::size_t which = 0; which < 3U; ++which) {
        const auto g = static_cast<std::uint8_t>(
            coordinate_code(state.cursor)
            + static_cast<std::uint8_t>(which * 29U)
            + static_cast<std::uint8_t>(state.symbol_index)
            + static_cast<std::uint8_t>(axis_code(state.previous_axis) * 11U));
        const auto mixed = static_cast<std::uint8_t>(
            mul_odd_byte(probe0, 29U)
            ^ mul_odd_byte(context_seed, 45U)
            ^ rotate_left_byte(g, static_cast<unsigned>(which & 7U))
            ^ static_cast<std::uint8_t>(which * 19U));
        c_digits[which] = static_cast<std::uint8_t>(mixed % 7U);
    }

    // Π(s) = (s * 41 + 17) mod 256 — public odd-mul permutation.
    const auto t = static_cast<std::uint8_t>(
        (static_cast<unsigned>(symbol) * 41U + 17U) & 0xFFU);
    const std::array<std::uint8_t, 3> d_digits{{
        static_cast<std::uint8_t>(t % 7U),
        static_cast<std::uint8_t>((t / 7U) % 7U),
        static_cast<std::uint8_t>(t / 49U),
    }};
    std::array<std::uint8_t, 3> inj_amounts{};
    for (std::size_t i = 0; i < 3U; ++i) {
        inj_amounts[i] = static_cast<std::uint8_t>(
            1U + ((static_cast<unsigned>(d_digits[i])
                 + static_cast<unsigned>(c_digits[i]))
                % 7U));
    }

    // Diffusion control (may depend on symbol); separate from c_i.
    const auto coord0 = coordinate_code(state.cursor);
    std::uint8_t control = static_cast<std::uint8_t>(
        rotate_left_byte(symbol, 3U)
        ^ rotate_left_byte(symbol, 1U)
        ^ mul_odd_byte(symbol, 5U)
        ^ static_cast<std::uint8_t>(state.symbol_index)
        ^ coord0
        ^ rotate_left_byte(coord0, 2U));

    for (std::uint8_t phase = 0; phase < 6U; ++phase) {
        const auto intersection_before = state.cursor;
        const auto probe_before = state.cube.at(state.cursor);
        const auto index_byte = static_cast<std::uint8_t>(
            state.symbol_index >> static_cast<unsigned>((phase & 7U) * 8U));
        const auto geometry = static_cast<std::uint8_t>(
            coordinate_code(state.cursor)
            + static_cast<std::uint8_t>(phase * 29U)
            + index_byte);

        auto sel = static_cast<std::uint8_t>(
            rotate_left_byte(symbol, phase)
            ^ rotate_left_byte(control, static_cast<unsigned>((phase + 1U) & 7U))
            ^ rotate_left_byte(probe_before, 2U)
            ^ geometry
            ^ static_cast<std::uint8_t>(axis_code(state.previous_axis) * 0x1DU)
            ^ static_cast<std::uint8_t>(phase * 0x3BU));
        sel = static_cast<std::uint8_t>(
            sel
            ^ rotate_left_byte(sel, 3U)
            ^ mul_odd_byte(symbol, 9U));
        const Axis axis = choose_other_axis(state.previous_axis, sel);

        std::uint8_t amount = 0;
        if (phase < 3U) {
            amount = inj_amounts[phase];
        } else {
            auto lane = static_cast<std::uint8_t>(
                mul_odd_byte(symbol, 73U)
                ^ mul_odd_byte(control, 45U)
                ^ mul_odd_byte(probe_before, 29U)
                ^ rotate_left_byte(geometry, phase & 7U)
                ^ static_cast<std::uint8_t>(phase * 19U)
                ^ static_cast<std::uint8_t>(axis_code(axis) * 11U));
            lane = static_cast<std::uint8_t>(
                lane
                + rotate_left_byte(lane, 4U)
                + rotate_left_byte(static_cast<std::uint8_t>(symbol ^ control), 2U));
            amount = static_cast<std::uint8_t>(
                1U
                + ((mul_odd_byte(lane, 41U)
                    ^ static_cast<std::uint8_t>(lane >> 3U)
                    ^ static_cast<std::uint8_t>(lane >> 5U)
                    ^ phase)
                   % 7U));
        }

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
            ^ mul_odd_byte(symbol, 3U)
            ^ probe_after
            ^ static_cast<std::uint8_t>(amount * 17U)
            ^ coordinate_code(state.cursor)
            ^ static_cast<std::uint8_t>(phase * 13U));

        state.previous_axis = axis;
    }

    ++state.symbol_index;
}

void absorb_symbol(WorkingState& state,
                   std::uint8_t symbol,
                   const HashParameters& parameters) {
    if (parameters.systematic_absorb) {
        if (parameters.moves_per_symbol != kMovesPerSymbol) {
            throw std::invalid_argument(
                "systematic absorb requires moves_per_symbol == 6");
        }
        absorb_symbol_systematic(state, symbol);
        return;
    }

    std::uint8_t control = static_cast<std::uint8_t>(
        symbol
        + static_cast<std::uint8_t>(state.symbol_index)
        + coordinate_code(state.cursor));

    for (std::size_t phase_index = 0;
         phase_index < parameters.moves_per_symbol;
         ++phase_index) {
        const auto phase = static_cast<std::uint8_t>(phase_index & 0xFFU);
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

        const auto amount = static_cast<std::uint8_t>(1U + (amount_source % 7U));

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

void absorb_forward(WorkingState& state,
                    std::span<const std::uint8_t> bytes,
                    const HashParameters& parameters) {
    for (const auto byte : bytes) {
        absorb_symbol(state, byte, parameters);
    }
}

std::uint8_t derive_return_symbol(std::uint8_t byte,
                                  std::size_t original_index) {
    const auto canonical_diagonals = Cube::perfect().body_diagonals();
    return static_cast<std::uint8_t>(
        byte
        ^ canonical_diagonals[original_index & 31U]
        ^ static_cast<std::uint8_t>(original_index * 8U
                                  + (original_index >> 3U)));
}

void absorb_foldback(WorkingState& state,
                     std::span<const std::uint8_t> bytes,
                     const HashParameters& parameters) {
    if (!parameters.enable_foldback) {
        return;
    }

    for (std::size_t reverse = bytes.size(); reverse > 0U; --reverse) {
        const std::size_t original_index = reverse - 1U;
        absorb_symbol(
            state, derive_return_symbol(bytes[original_index], original_index), parameters);
    }
}

ResearchHashResult compute(std::span<const std::uint8_t> bytes,
                           const HashParameters& parameters,
                           bool keep_trace,
                           bool keep_checkpoints) {
    validate_hash_parameters(parameters);

    WorkingState state;
    std::vector<Move> trace;
    std::vector<StateCheckpoint> checkpoints;
    auto* checkpoint_target = keep_checkpoints ? &checkpoints : nullptr;

    if (keep_trace) {
        const auto foldback_symbols = parameters.enable_foldback ? bytes.size() : 0U;
        const auto symbol_count = bytes.size()
                                + foldback_symbols
                                + 2U + 8U
                                + parameters.diagonal_closure_symbols
                                + parameters.orbit_closure_symbols
                                + parameters.squeeze_bytes
                                    * parameters.squeeze_symbols_per_byte;
        if (symbol_count > std::numeric_limits<std::size_t>::max()
                         / parameters.moves_per_symbol) {
            throw std::length_error("research trace size overflows size_t");
        }
        trace.reserve(symbol_count * parameters.moves_per_symbol);
        state.trace = &trace;
    }

    add_checkpoint(checkpoint_target, ResearchPhase::Initial, 0U, state);

    absorb_forward(state, bytes, parameters);
    add_checkpoint(checkpoint_target, ResearchPhase::AfterForward, bytes.size(), state);

    absorb_foldback(state, bytes, parameters);
    add_checkpoint(checkpoint_target, ResearchPhase::AfterFoldback, bytes.size(), state);

    const auto input_size = static_cast<std::uint64_t>(bytes.size());
    absorb_closure(state, input_size, parameters);
    add_checkpoint(checkpoint_target,
                   ResearchPhase::AfterDiagonalClosure,
                   parameters.diagonal_closure_symbols,
                   state);

    absorb_orbit_closure(state, input_size, parameters);
    add_checkpoint(checkpoint_target,
                   ResearchPhase::AfterOrbitClosure,
                   parameters.orbit_closure_symbols,
                   state);

    ResearchHashResult result;
    result.digest = squeeze_diagonals(state,
                                      input_size,
                                      parameters,
                                      checkpoint_target);
    add_checkpoint(checkpoint_target,
                   ResearchPhase::Final,
                   result.digest.size(),
                   state);

    result.final_state = snapshot(state);
    result.input_size = input_size;
    if (keep_trace) {
        result.trace = std::move(trace);
    }
    if (keep_checkpoints) {
        result.checkpoints = std::move(checkpoints);
    }
    return result;
}

} // namespace pvc::detail
