#include "alias_catalog.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t prefix_count = 65536U;
    std::size_t threads = 0U;
    std::size_t print_limit = 8U;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--prefix-count" && i + 1 < argc) {
            options.prefix_count = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            options.threads = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-foldback-separation-profile [--preset NAME] "
                "[--prefix-count 1..65536] [--threads N] [--print-limit N]");
        }
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    return options;
}

std::size_t highest_differing_index(const pvc::tool::ForwardCollisionPair& pair) {
    for (std::size_t reverse = pair.left.size(); reverse > 0U; --reverse) {
        const auto index = reverse - 1U;
        if (pair.left[index] != pair.right[index]) {
            return index;
        }
    }
    throw std::logic_error("collision pair contains identical messages");
}

struct DistanceStats {
    std::size_t minimum = std::numeric_limits<std::size_t>::max();
    std::size_t maximum = 0U;
    long double total = 0.0L;
    std::size_t samples = 0U;

    void add(std::size_t value) {
        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
        total += static_cast<long double>(value);
        ++samples;
    }

    [[nodiscard]] long double mean() const {
        return samples == 0U ? 0.0L : total / static_cast<long double>(samples);
    }
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto pairs = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);

        std::cout << "Foldback separation profile\n"
                  << "preset=" << preset.name
                  << " prefix_count=" << options.prefix_count
                  << " forward_collision_pairs=" << pairs.size() << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::size_t return_difference_preserved = 0U;
        std::size_t first_divergence_at_expected_step = 0U;
        std::size_t delayed_divergence = 0U;
        std::size_t never_diverged = 0U;
        std::size_t direct_return_aliases = 0U;
        std::size_t later_reconvergences = 0U;
        std::size_t final_merges = 0U;
        std::size_t cursor_diff_at_gate = 0U;
        std::size_t axis_diff_at_gate = 0U;
        std::size_t cursor_diff_final = 0U;
        std::size_t axis_diff_final = 0U;
        std::array<std::uint64_t, 256> absolute_delta_histogram{};
        std::array<std::uint64_t, 4> divergence_step_histogram{};
        DistanceStats gate_bit_distance;
        DistanceStats gate_byte_distance;
        DistanceStats final_bit_distance;
        DistanceStats final_byte_distance;
        std::size_t printed = 0U;

        for (const auto& pair : pairs) {
            auto left_state = pair.common_state;
            auto right_state = pair.common_state;
            const auto differing_index = highest_differing_index(pair);
            const auto expected_step = pair.left.size() - differing_index;
            const auto left_byte = pair.left[differing_index];
            const auto right_byte = pair.right[differing_index];
            const auto absolute_delta = left_byte > right_byte
                ? static_cast<std::uint8_t>(left_byte - right_byte)
                : static_cast<std::uint8_t>(right_byte - left_byte);
            ++absolute_delta_histogram[absolute_delta];

            const auto left_gate_symbol = pvc::return_symbol_for_research(
                left_byte, differing_index);
            const auto right_gate_symbol = pvc::return_symbol_for_research(
                right_byte, differing_index);
            if (static_cast<std::uint8_t>(left_gate_symbol ^ right_gate_symbol)
                == static_cast<std::uint8_t>(left_byte ^ right_byte)) {
                ++return_difference_preserved;
            }

            std::size_t first_divergence_step = 0U;
            bool reconverged = false;
            for (std::size_t reverse = pair.left.size(); reverse > 0U; --reverse) {
                const auto original_index = reverse - 1U;
                const auto left_symbol = pvc::return_symbol_for_research(
                    pair.left[original_index], original_index);
                const auto right_symbol = pvc::return_symbol_for_research(
                    pair.right[original_index], original_index);
                const bool equal_before = left_state == right_state;
                left_state = pvc::absorb_symbol_for_research(
                    left_state, left_symbol, preset.parameters);
                right_state = pvc::absorb_symbol_for_research(
                    right_state, right_symbol, preset.parameters);
                const bool equal_after = left_state == right_state;
                const auto step = pair.left.size() - original_index;

                if (equal_before && left_symbol != right_symbol && equal_after) {
                    ++direct_return_aliases;
                }
                if (equal_before && !equal_after && first_divergence_step == 0U) {
                    first_divergence_step = step;
                    gate_bit_distance.add(pvc::tool::cube_bit_distance(
                        left_state.cube, right_state.cube));
                    gate_byte_distance.add(pvc::tool::cube_byte_distance(
                        left_state.cube, right_state.cube));
                    if (left_state.cursor != right_state.cursor) {
                        ++cursor_diff_at_gate;
                    }
                    if (left_state.previous_axis != right_state.previous_axis) {
                        ++axis_diff_at_gate;
                    }
                }
                if (!equal_before && equal_after) {
                    reconverged = true;
                }
            }

            if (first_divergence_step == 0U) {
                ++never_diverged;
            } else {
                if (first_divergence_step < divergence_step_histogram.size()) {
                    ++divergence_step_histogram[first_divergence_step];
                }
                if (first_divergence_step == expected_step) {
                    ++first_divergence_at_expected_step;
                } else {
                    ++delayed_divergence;
                }
            }
            if (reconverged) {
                ++later_reconvergences;
            }
            if (left_state == right_state) {
                ++final_merges;
            } else {
                final_bit_distance.add(pvc::tool::cube_bit_distance(
                    left_state.cube, right_state.cube));
                final_byte_distance.add(pvc::tool::cube_byte_distance(
                    left_state.cube, right_state.cube));
                if (left_state.cursor != right_state.cursor) {
                    ++cursor_diff_final;
                }
                if (left_state.previous_axis != right_state.previous_axis) {
                    ++axis_diff_final;
                }
            }

            if (printed < options.print_limit) {
                std::cout << "pair[" << printed << "]="
                          << pvc::tool::hex_bytes(pvc::tool::as_vector(pair.left))
                          << '/'
                          << pvc::tool::hex_bytes(pvc::tool::as_vector(pair.right))
                          << " differing_index=" << differing_index
                          << " expected_reverse_step=" << expected_step
                          << " return_symbols=" << std::hex
                          << static_cast<unsigned>(left_gate_symbol) << '/'
                          << static_cast<unsigned>(right_gate_symbol) << std::dec
                          << " first_divergence_step=" << first_divergence_step
                          << " final_cube_bits="
                          << pvc::tool::cube_bit_distance(
                                 left_state.cube, right_state.cube)
                          << '\n';
                ++printed;
            }
        }

        std::cout << std::fixed << std::setprecision(4)
                  << "return_xor_difference_preserved="
                  << return_difference_preserved << '\n'
                  << "first_divergence_at_expected_step="
                  << first_divergence_at_expected_step << '\n'
                  << "delayed_divergence=" << delayed_divergence << '\n'
                  << "never_diverged=" << never_diverged << '\n'
                  << "direct_return_transition_aliases="
                  << direct_return_aliases << '\n'
                  << "later_reconvergences=" << later_reconvergences << '\n'
                  << "final_after_foldback_merges=" << final_merges << '\n'
                  << "gate_cube_bit_distance_min=" << gate_bit_distance.minimum << '\n'
                  << "gate_cube_bit_distance_mean=" << gate_bit_distance.mean() << '\n'
                  << "gate_cube_bit_distance_max=" << gate_bit_distance.maximum << '\n'
                  << "gate_cube_byte_distance_min=" << gate_byte_distance.minimum << '\n'
                  << "gate_cube_byte_distance_mean=" << gate_byte_distance.mean() << '\n'
                  << "gate_cube_byte_distance_max=" << gate_byte_distance.maximum << '\n'
                  << "final_cube_bit_distance_min=" << final_bit_distance.minimum << '\n'
                  << "final_cube_bit_distance_mean=" << final_bit_distance.mean() << '\n'
                  << "final_cube_bit_distance_max=" << final_bit_distance.maximum << '\n'
                  << "final_cube_byte_distance_min=" << final_byte_distance.minimum << '\n'
                  << "final_cube_byte_distance_mean=" << final_byte_distance.mean() << '\n'
                  << "final_cube_byte_distance_max=" << final_byte_distance.maximum << '\n'
                  << "cursor_different_at_gate=" << cursor_diff_at_gate << '\n'
                  << "axis_different_at_gate=" << axis_diff_at_gate << '\n'
                  << "cursor_different_final=" << cursor_diff_final << '\n'
                  << "axis_different_final=" << axis_diff_final << '\n'
                  << "reverse_step,count\n";
        for (std::size_t step = 1U; step < divergence_step_histogram.size(); ++step) {
            if (divergence_step_histogram[step] != 0U) {
                std::cout << step << ',' << divergence_step_histogram[step] << '\n';
            }
        }
        std::cout << "absolute_byte_delta,count\n";
        for (std::size_t delta = 1U; delta < absolute_delta_histogram.size(); ++delta) {
            if (absolute_delta_histogram[delta] != 0U) {
                std::cout << delta << ',' << absolute_delta_histogram[delta] << '\n';
            }
        }
        return final_merges == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
