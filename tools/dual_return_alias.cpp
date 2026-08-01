// Phase 1-b: search for foldback-compatible dual aliases on known forward
// collisions. Uses the frozen research API only; does not change the candidate.

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

constexpr std::array<std::uint8_t, 3> kFamilyDeltas{{42U, 126U, 196U}};

struct Options {
    std::string preset = "R5-canonical";
    std::size_t prefix_count = 65536U;
    std::size_t threads = 0U;
    std::size_t suffix_bytes = 1U;
    std::uint64_t suffix_limit = 256U;
    std::size_t print_limit = 8U;
    bool family_surface = true;
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
        } else if (arg == "--no-family-surface") {
            options.family_surface = false;
        } else {
            throw std::invalid_argument(
                "usage: pvc-dual-return-alias [--preset NAME] "
                "[--prefix-count 1..65536] [--threads N] "
                "[--suffix-bytes 0..2] [--suffix-limit N] "
                "[--print-limit N] [--no-family-surface]");
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

bool is_family_delta(std::uint8_t delta) {
    return std::find(kFamilyDeltas.begin(), kFamilyDeltas.end(), delta)
        != kFamilyDeltas.end();
}

std::uint8_t abs_delta(std::uint8_t left, std::uint8_t right) {
    return left > right ? static_cast<std::uint8_t>(left - right)
                        : static_cast<std::uint8_t>(right - left);
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

        std::cout << "Dual return-alias search (Phase 1-b)\n"
                  << "preset=" << preset.name
                  << " prefix_count=" << options.prefix_count
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffix_domain=" << suffix_domain
                  << " family_surface=" << (options.family_surface ? "yes" : "no")
                  << '\n';
        pvc::tool::print_parameters(preset.parameters);

        const auto pairs = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);
        std::cout << "forward_collision_pairs=" << pairs.size() << '\n';

        std::uint64_t direct_return_merges = 0U;
        std::uint64_t direct_family_xor_preserved = 0U;
        std::uint64_t full_foldback_merges = 0U;
        std::uint64_t return_gate_cases = 0U;
        std::size_t min_return_bits = std::numeric_limits<std::size_t>::max();
        std::size_t min_foldback_bits = std::numeric_limits<std::size_t>::max();
        std::size_t printed = 0U;

        std::array<std::uint64_t, 256> return_delta_hist{};
        std::uint64_t family_surface_contexts = 0U;
        std::uint64_t family_surface_alias_contexts = 0U;
        std::uint64_t family_surface_alias_pairs = 0U;
        std::array<std::uint64_t, 256> family_surface_delta_hist{};

        for (const auto& pair : pairs) {
            const auto left_byte = pair.left[pair.differing_index];
            const auto right_byte = pair.right[pair.differing_index];
            const auto msg_delta = abs_delta(left_byte, right_byte);
            const auto left_return = pvc::return_symbol_for_research(
                left_byte, pair.differing_index);
            const auto right_return = pvc::return_symbol_for_research(
                right_byte, pair.differing_index);
            const auto ret_delta = abs_delta(left_return, right_return);
            ++return_delta_hist[ret_delta];
            if (ret_delta == msg_delta) {
                ++direct_family_xor_preserved;
            }

            // Direct dual: same common forward state, one reverse step on returns.
            const auto left_gate = pvc::absorb_symbol_for_research(
                pair.common_state, left_return, preset.parameters);
            const auto right_gate = pvc::absorb_symbol_for_research(
                pair.common_state, right_return, preset.parameters);
            ++return_gate_cases;
            const auto gate_bits = pvc::tool::operational_state_bit_distance(
                left_gate, right_gate);
            min_return_bits = std::min(min_return_bits, gate_bits);
            if (left_gate == right_gate) {
                ++direct_return_merges;
                if (printed < options.print_limit) {
                    std::cout << "direct_return_merge left="
                              << pvc::tool::hex_bytes(pvc::tool::as_vector(pair.left))
                              << " right="
                              << pvc::tool::hex_bytes(pvc::tool::as_vector(pair.right))
                              << " msg_delta=" << static_cast<unsigned>(msg_delta)
                              << " ret_delta=" << static_cast<unsigned>(ret_delta)
                              << '\n';
                    ++printed;
                }
            }

            // Full foldback for bare pair and common suffixes.
            for (std::uint64_t suffix_counter = 0U;
                 suffix_counter < suffix_domain;
                 ++suffix_counter) {
                const auto suffix = suffix_from_counter(
                    suffix_counter, options.suffix_bytes);
                const auto left_message = extended_message(pair.left, suffix);
                const auto right_message = extended_message(pair.right, suffix);
                const auto left_fb = pvc::foldback_state_for_research(
                    left_message, preset.parameters);
                const auto right_fb = pvc::foldback_state_for_research(
                    right_message, preset.parameters);
                const auto fb_bits = pvc::tool::operational_state_bit_distance(
                    left_fb, right_fb);
                min_foldback_bits = std::min(min_foldback_bits, fb_bits);
                if (left_fb == right_fb) {
                    ++full_foldback_merges;
                    if (printed < options.print_limit) {
                        std::cout << "exact_after_foldback_merge left="
                                  << pvc::tool::hex_bytes(left_message)
                                  << " right=" << pvc::tool::hex_bytes(right_message)
                                  << '\n';
                        ++printed;
                    }
                }
            }

            if (options.family_surface) {
                // From the common forward state, any one-symbol controller alias
                // with a family delta is a potential reverse-context dual engine.
                ++family_surface_contexts;
                const auto aliases = pvc::tool::find_symbol_aliases(
                    pair.common_state, preset.parameters);
                bool context_hit = false;
                for (const auto& alias : aliases) {
                    const auto d = abs_delta(alias.left, alias.right);
                    ++family_surface_delta_hist[d];
                    if (is_family_delta(d)) {
                        context_hit = true;
                        ++family_surface_alias_pairs;
                    }
                }
                if (context_hit) {
                    ++family_surface_alias_contexts;
                }
            }
        }

        std::cout << "direct_return_gate_cases=" << return_gate_cases << '\n'
                  << "direct_return_merges=" << direct_return_merges << '\n'
                  << "return_xor_delta_equals_message_delta="
                  << direct_family_xor_preserved << '\n'
                  << "suffix_domain_per_pair=" << suffix_domain << '\n'
                  << "exact_after_foldback_merges=" << full_foldback_merges << '\n'
                  << "minimum_direct_return_state_bits=" << min_return_bits << '\n'
                  << "minimum_after_foldback_state_bits=" << min_foldback_bits << '\n';

        if (options.family_surface) {
            std::cout << "family_surface_contexts=" << family_surface_contexts << '\n'
                      << "family_surface_contexts_with_family_delta_alias="
                      << family_surface_alias_contexts << '\n'
                      << "family_surface_family_delta_alias_pairs="
                      << family_surface_alias_pairs << '\n'
                      << "note=family_surface counts any one-symbol alias from the "
                         "common forward state; it is not itself a message collision\n";
        }

        std::cout << "return_symbol_abs_delta,hits\n";
        for (std::size_t d = 0; d < return_delta_hist.size(); ++d) {
            if (return_delta_hist[d] != 0U) {
                std::cout << d << ',' << return_delta_hist[d] << '\n';
            }
        }
        if (options.family_surface) {
            std::cout << "forward_context_alias_abs_delta,hits\n";
            for (std::size_t d = 0; d < family_surface_delta_hist.size(); ++d) {
                if (family_surface_delta_hist[d] != 0U) {
                    std::cout << d << ',' << family_surface_delta_hist[d] << '\n';
                }
            }
        }

        return full_foldback_merges == 0U && direct_return_merges == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
