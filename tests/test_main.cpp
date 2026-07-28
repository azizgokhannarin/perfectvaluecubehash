#include "pvc/cube.hpp"
#include "pvc/hash.hpp"
#include "pvc/hex.hpp"

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
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
    const auto empty_a = pvc::RotHash0::hash(std::string_view{});
    const auto empty_b = pvc::RotHash0::hash(std::string_view{});
    check(empty_a == empty_b, "empty input is deterministic");

    const auto abc_a = pvc::RotHash0::hash("abc");
    const auto abc_b = pvc::RotHash0::hash("abc");
    check(abc_a == abc_b, "same message is deterministic");

    const std::array<std::uint8_t, 1> a{{0x61}};
    const std::array<std::uint8_t, 2> a_zero{{0x61, 0x00}};
    check(pvc::RotHash0::hash(a) != pvc::RotHash0::hash(a_zero),
          "length framing distinguishes a from a||00");

    check(empty_a != abc_a, "different messages produce different sample digests");
}

void test_trace_chain() {
    const std::vector<std::uint8_t> input{'c', 'h', 'a', 'i', 'n'};
    const auto result = pvc::RotHash0::inspect(input, true);

    check(!result.trace.empty(), "trace contains moves");
    check(result.trace.size()
              == (input.size() * 2U + 2U + 8U + pvc::kClosureSymbols) * pvc::kMovesPerSymbol,
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
          "rotation-only hash preserves the byte histogram");
}

void print_known_answers() {
    std::cout << "KAT empty: " << pvc::to_hex(pvc::RotHash0::hash("")) << '\n';
    std::cout << "KAT abc  : " << pvc::to_hex(pvc::RotHash0::hash("abc")) << '\n';
}

} // namespace

int main() {
    try {
        test_perfect_cube();
        test_rotation_inverse();
        test_hash_determinism_and_framing();
        test_trace_chain();
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
