#include "alias_catalog.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t prefix_count = 65536U;
    std::size_t threads = 0U;
    std::size_t suffix_bytes = 1U;
    std::uint64_t suffix_limit = 256U;
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
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-limit" && i + 1 < argc) {
            options.suffix_limit = std::stoull(argv[++i]);
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-foldback-aware-alias [--preset NAME] "
                "[--prefix-count 1..65536] [--threads N] "
                "[--suffix-bytes 0..2] [--suffix-limit N] [--print-limit N]");
        }
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    if (options.suffix_bytes > 2U) {
        throw std::invalid_argument("suffix-bytes must be in [0,2]");
    }
    return options;
}

std::vector<std::uint8_t> suffix_from_counter(std::uint64_t counter,
                                              std::size_t bytes) {
    std::vector<std::uint8_t> suffix(bytes);
    for (std::size_t i = 0; i < bytes; ++i) {
        const auto shift = static_cast<unsigned>((bytes - 1U - i) * 8U);
        suffix[i] = static_cast<std::uint8_t>((counter >> shift) & 0xFFU);
    }
    return suffix;
}

std::vector<std::uint8_t> extended_message(
    const std::array<std::uint8_t, 3>& prefix,
    const std::vector<std::uint8_t>& suffix) {
    std::vector<std::uint8_t> result(prefix.begin(), prefix.end());
    result.insert(result.end(), suffix.begin(), suffix.end());
    return result;
}

pvc::InternalStateSnapshot apply_common_extension_and_reverse(
    const pvc::tool::ForwardCollisionPair& pair,
    const std::vector<std::uint8_t>& suffix,
    const pvc::HashParameters& parameters) {
    auto state = pair.common_state;
    for (const auto byte : suffix) {
        state = pvc::absorb_symbol_for_research(state, byte, parameters);
    }

    for (std::size_t reverse = suffix.size(); reverse > 0U; --reverse) {
        const auto suffix_index = reverse - 1U;
        const auto original_index = 3U + suffix_index;
        const auto symbol = pvc::return_symbol_for_research(
            suffix[suffix_index], original_index);
        state = pvc::absorb_symbol_for_research(state, symbol, parameters);
    }

    for (std::size_t reverse = 3U; reverse > pair.differing_index + 1U; --reverse) {
        const auto original_index = reverse - 1U;
        const auto symbol = pvc::return_symbol_for_research(
            pair.left[original_index], original_index);
        state = pvc::absorb_symbol_for_research(state, symbol, parameters);
    }
    return state;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const std::uint64_t full_suffix_domain = options.suffix_bytes == 0U
            ? std::uint64_t{1}
            : (std::uint64_t{1} << static_cast<unsigned>(options.suffix_bytes * 8U));
        const auto suffix_domain = std::min(full_suffix_domain, options.suffix_limit);

        std::cout << "Foldback-aware controller alias search\n"
                  << "preset=" << preset.name
                  << " prefix_count=" << options.prefix_count
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffix_domain=" << suffix_domain << '\n';
        pvc::tool::print_parameters(preset.parameters);

        const auto pairs = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);
        std::size_t inherited = 0U;
        std::array<std::uint64_t, 256> delta_histogram{};
        for (const auto& pair : pairs) {
            inherited += pair.inherited ? 1U : 0U;
            const auto left = pair.left[pair.differing_index];
            const auto right = pair.right[pair.differing_index];
            const auto delta = static_cast<std::uint8_t>(right - left);
            ++delta_histogram[delta];
        }

        std::uint64_t tested = 0U;
        std::uint64_t return_transition_merges = 0U;
        std::uint64_t full_foldback_merges = 0U;
        std::size_t minimum_bits = std::numeric_limits<std::size_t>::max();
        std::size_t minimum_bytes = std::numeric_limits<std::size_t>::max();
        std::size_t printed = 0U;
        std::vector<std::uint8_t> minimum_left;
        std::vector<std::uint8_t> minimum_right;

        for (const auto& pair : pairs) {
            for (std::uint64_t suffix_counter = 0U;
                 suffix_counter < suffix_domain;
                 ++suffix_counter) {
                const auto suffix = suffix_from_counter(
                    suffix_counter, options.suffix_bytes);
                const auto before_difference = apply_common_extension_and_reverse(
                    pair, suffix, preset.parameters);
                const auto left_return = pvc::return_symbol_for_research(
                    pair.left[pair.differing_index], pair.differing_index);
                const auto right_return = pvc::return_symbol_for_research(
                    pair.right[pair.differing_index], pair.differing_index);
                const auto left_state = pvc::absorb_symbol_for_research(
                    before_difference, left_return, preset.parameters);
                const auto right_state = pvc::absorb_symbol_for_research(
                    before_difference, right_return, preset.parameters);
                ++tested;

                const auto bits = pvc::tool::cube_bit_distance(
                    left_state.cube, right_state.cube);
                const auto bytes = pvc::tool::cube_byte_distance(
                    left_state.cube, right_state.cube);
                if (bits < minimum_bits || (bits == minimum_bits && bytes < minimum_bytes)) {
                    minimum_bits = bits;
                    minimum_bytes = bytes;
                    minimum_left = extended_message(pair.left, suffix);
                    minimum_right = extended_message(pair.right, suffix);
                }

                if (left_state != right_state) {
                    continue;
                }
                ++return_transition_merges;

                const auto left_message = extended_message(pair.left, suffix);
                const auto right_message = extended_message(pair.right, suffix);
                const auto left_full = pvc::foldback_from_forward_state_for_research(
                    pvc::forward_state_for_research(left_message, preset.parameters),
                    left_message,
                    preset.parameters);
                const auto right_full = pvc::foldback_from_forward_state_for_research(
                    pvc::forward_state_for_research(right_message, preset.parameters),
                    right_message,
                    preset.parameters);
                if (left_full == right_full) {
                    ++full_foldback_merges;
                    if (printed < options.print_limit) {
                        std::cout << "exact_foldback_aware_merge left="
                                  << pvc::tool::hex_bytes(left_message)
                                  << " right=" << pvc::tool::hex_bytes(right_message)
                                  << '\n';
                        ++printed;
                    }
                }
            }
        }

        std::cout << "forward_collision_pairs=" << pairs.size() << '\n'
                  << "local_alias_pairs=" << (pairs.size() - inherited) << '\n'
                  << "inherited_pairs=" << inherited << '\n'
                  << "extension_cases_tested=" << tested << '\n'
                  << "return_transition_merges=" << return_transition_merges << '\n'
                  << "exact_after_foldback_merges=" << full_foldback_merges << '\n'
                  << "minimum_return_cube_bit_distance=" << minimum_bits << '\n'
                  << "minimum_return_cube_byte_distance=" << minimum_bytes << '\n'
                  << "minimum_left=" << pvc::tool::hex_bytes(minimum_left) << '\n'
                  << "minimum_right=" << pvc::tool::hex_bytes(minimum_right) << '\n'
                  << "symbol_delta,hits\n";
        for (std::size_t delta = 0U; delta < delta_histogram.size(); ++delta) {
            if (delta_histogram[delta] != 0U) {
                std::cout << delta << ',' << delta_histogram[delta] << '\n';
            }
        }
        return full_foldback_merges == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
