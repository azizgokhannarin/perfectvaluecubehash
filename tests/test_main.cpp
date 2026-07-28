#include "pvc/cube.hpp"
#include "pvc/hash.hpp"
#include "pvc/hex.hpp"
#include "pvc/research.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, std::string_view description) {
    if (!condition) {
        ++failures;
        std::cerr << "FAIL: " << description << '\n';
    }
}

void test_perfect_cube() {
    const auto cube = pvc::Cube::perfect();
    check(cube.is_balanced(), "canonical cube has all 192 line sums equal to 1020");
    check(cube.has_double_byte_histogram(), "canonical cube contains each byte exactly twice");

    const auto diagonals = cube.body_diagonals();
    check(diagonals.size() == 32U, "four body diagonals contain 32 bytes");
}

void test_rotation_inverse() {
    const auto original = pvc::Cube::perfect();

    for (const auto axis : {pvc::Axis::X, pvc::Axis::Y, pvc::Axis::Z}) {
        for (std::uint8_t amount = 1; amount <= 7U; ++amount) {
            auto cube = original;
            const pvc::Coord point{3, 5, 6};
            cube.rotate_line(axis, point, amount);
            cube.rotate_line(axis, point, static_cast<std::uint8_t>(8U - amount));
            check(cube == original, "line rotation followed by inverse restores cube");
        }

        auto cube = original;
        const pvc::Coord point{2, 4, 7};
        for (int i = 0; i < 8; ++i) {
            cube.rotate_line(axis, point, 1);
        }
        check(cube == original, "eight unit rotations restore cube");
    }
}

void test_hash_determinism_and_framing() {
    const auto empty_a = pvc::RotHash1::hash(std::string_view{});
    const auto empty_b = pvc::RotHash1::hash(std::string_view{});
    check(empty_a == empty_b, "empty input is deterministic");
    check(pvc::to_hex(empty_a)
              == "7f01eb3ce13131ef290f8428ed725b849f875e49ad6c646cc9f4f1b1a1e5734b",
          "empty-input regression vector matches");

    const auto abc_a = pvc::RotHash1::hash("abc");
    const auto abc_b = pvc::RotHash1::hash("abc");
    check(abc_a == abc_b, "same message is deterministic");
    check(pvc::to_hex(abc_a)
              == "f32b2241a950d7e7b2b006ff8ae2d0b08f02db23c0d8fde198dfdf9e9642051f",
          "abc regression vector matches");

    const std::array<std::uint8_t, 1> a{{0x61}};
    const std::array<std::uint8_t, 2> a_zero{{0x61, 0x00}};
    check(pvc::RotHash1::hash(a) != pvc::RotHash1::hash(a_zero),
          "length framing distinguishes a from a||00");

    check(empty_a != abc_a, "different messages produce different sample digests");
}

void test_trace_chain() {
    const std::vector<std::uint8_t> input{'c', 'h', 'a', 'i', 'n'};
    const auto result = pvc::RotHash1::inspect(input, true);

    check(!result.trace.empty(), "trace contains moves");
    const auto expected_symbols = input.size() * 2U
                                + 2U + 8U
                                + pvc::kClosureSymbols
                                + pvc::kOrbitSymbols
                                + pvc::kDigestBytes * pvc::kSqueezeSymbolsPerByte;
    check(result.trace.size() == expected_symbols * pvc::kMovesPerSymbol,
          "trace has expected move count");

    for (std::size_t i = 1; i < result.trace.size(); ++i) {
        const auto& previous = result.trace[i - 1U];
        const auto& current = result.trace[i];

        check(previous.axis != current.axis,
              "consecutive moves use different axes");
        check(pvc::line_contains(previous.axis,
                                 previous.intersection_before,
                                 current.intersection_before),
              "current move starts at a point on previous move line");
    }

    check(result.final_cube.has_double_byte_histogram(),
          "rotation-only state mutation preserves the byte histogram");
    check(result.digest != result.final_cube.body_diagonals(),
          "digest is a diagonal squeeze, not a raw final-state projection");
}


void test_research_framework() {
    const std::vector<std::uint8_t> abc{'a', 'b', 'c'};
    const auto canonical = pvc::canonical_hash_parameters();
    const auto research = pvc::inspect_with_parameters(abc, canonical, false, true);
    const auto digest = pvc::RotHash1::hash(abc);

    check(research.digest.size() == pvc::kDigestBytes,
          "canonical research digest has 32 bytes");
    check(std::equal(research.digest.begin(), research.digest.end(), digest.begin()),
          "parameterized canonical path matches RotHash1");
    check(research.checkpoints.size() == 38U,
          "canonical research path records phase endpoints and 32 squeeze states");
    check(research.checkpoints.front().phase == pvc::ResearchPhase::Initial,
          "first research checkpoint is initial state");
    check(research.checkpoints.back().phase == pvc::ResearchPhase::Final,
          "last research checkpoint is final state");

    const auto start = pvc::initial_internal_state();
    const auto next = pvc::absorb_symbol_for_research(start, 0x42U, canonical);
    check(next.symbol_index == 1U,
          "single-symbol research transition increments symbol index");
    check(next.cube.has_double_byte_histogram(),
          "single-symbol research transition preserves cube histogram");

    const auto presets = pvc::reduced_round_presets();
    check(presets.size() == 6U, "six reduced-round presets are available");
    check(presets.back().parameters == canonical,
          "last reduced-round preset is canonical");
}

const pvc::InternalStateSnapshot& checkpoint_for(
    const pvc::ResearchHashResult& result,
    pvc::ResearchPhase phase) {
    if (phase == pvc::ResearchPhase::Final) {
        return result.final_state;
    }
    for (const auto& checkpoint : result.checkpoints) {
        if (checkpoint.phase == phase) {
            return checkpoint.state;
        }
    }
    throw std::logic_error("checkpoint missing in test");
}

void test_forward_collision_foldback_regression() {
    const std::array<std::uint8_t, 2> left{{0x17U, 0x6fU}};
    const std::array<std::uint8_t, 2> right{{0x17U, 0x99U}};
    const auto parameters = pvc::canonical_hash_parameters();

    const auto forward_left = pvc::forward_state_for_research(left, parameters);
    const auto forward_right = pvc::forward_state_for_research(right, parameters);
    check(forward_left == forward_right,
          "known canonical two-symbol forward convergence is reproducible");

    const auto full_left = pvc::inspect_with_parameters(left, parameters, false, true);
    const auto full_right = pvc::inspect_with_parameters(right, parameters, false, true);
    check(checkpoint_for(full_left, pvc::ResearchPhase::AfterFoldback)
              != checkpoint_for(full_right, pvc::ResearchPhase::AfterFoldback),
          "foldback separates the known forward convergence");
    check(full_left.final_state != full_right.final_state,
          "known forward convergence does not survive full finalization");
    check(full_left.digest != full_right.digest,
          "known forward convergence is not a digest collision");
}


bool same_physical_move(const pvc::Move& left, const pvc::Move& right) {
    return left.axis == right.axis
        && left.intersection_before == right.intersection_before
        && left.intersection_after == right.intersection_after
        && left.amount == right.amount
        && left.phase == right.phase
        && left.symbol_index == right.symbol_index;
}

void test_context_dependent_symbol_alias_regression() {
    const std::array<std::uint8_t, 2> prefix{{0xafU, 0x67U}};
    const auto parameters = pvc::canonical_hash_parameters();
    const auto context = pvc::forward_state_for_research(prefix, parameters);

    std::vector<pvc::Move> left_trace;
    std::vector<pvc::Move> right_trace;
    const auto left = pvc::absorb_symbol_for_research(
        context, 0x1bU, parameters, &left_trace);
    const auto right = pvc::absorb_symbol_for_research(
        context, 0xdfU, parameters, &right_trace);

    check(left == right,
          "known context-dependent symbol alias reaches the same forward state");
    check(left_trace.size() == pvc::kMovesPerSymbol
              && right_trace.size() == pvc::kMovesPerSymbol,
          "known symbol alias records one complete move sequence per symbol");
    bool same_trace = left_trace.size() == right_trace.size();
    for (std::size_t i = 0; same_trace && i < left_trace.size(); ++i) {
        same_trace = same_physical_move(left_trace[i], right_trace[i]);
    }
    check(same_trace,
          "known context-dependent symbol alias generates identical physical moves");

    const std::array<std::uint8_t, 3> left_message{{0xafU, 0x67U, 0x1bU}};
    const std::array<std::uint8_t, 3> right_message{{0xafU, 0x67U, 0xdfU}};
    check(pvc::foldback_state_for_research(left_message, parameters)
              != pvc::foldback_state_for_research(right_message, parameters),
          "foldback separates the known three-byte symbol alias");
}

void test_foldback_state_api() {
    const std::array<std::uint8_t, 3> message{{'a', 'b', 'c'}};
    const auto parameters = pvc::canonical_hash_parameters();
    const auto direct = pvc::foldback_state_for_research(message, parameters);
    const auto inspected = pvc::inspect_with_parameters(
        message, parameters, false, true);
    check(direct == checkpoint_for(inspected, pvc::ResearchPhase::AfterFoldback),
          "foldback-only research API matches the full phase checkpoint");
}

void print_known_answers() {
    std::cout << "KAT empty: " << pvc::to_hex(pvc::RotHash1::hash("")) << '\n';
    std::cout << "KAT abc  : " << pvc::to_hex(pvc::RotHash1::hash("abc")) << '\n';
}

} // namespace

int main() {
    try {
        test_perfect_cube();
        test_rotation_inverse();
        test_hash_determinism_and_framing();
        test_trace_chain();
        test_research_framework();
        test_forward_collision_foldback_regression();
        test_context_dependent_symbol_alias_regression();
        test_foldback_state_api();
        print_known_answers();

        if (failures != 0) {
            std::cerr << failures << " test(s) failed.\n";
            return 1;
        }

        std::cout << "All tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Unhandled exception: " << error.what() << '\n';
        return 1;
    }
}
