#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::string left_hex = "176f";
    std::string right_hex = "1799";
    std::size_t suffix_bytes = 2;
    std::uint64_t limit = 65536;
    std::size_t print_limit = 8;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--left" && i + 1 < argc) {
            options.left_hex = argv[++i];
        } else if (arg == "--right" && i + 1 < argc) {
            options.right_hex = argv[++i];
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--limit" && i + 1 < argc) {
            options.limit = std::stoull(argv[++i]);
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-constrained-merge-search [--preset NAME] "
                "[--left HEX] [--right HEX] [--suffix-bytes 1|2] "
                "[--limit N] [--print-limit N]");
        }
    }
    if (options.suffix_bytes == 0U || options.suffix_bytes > 2U) {
        throw std::invalid_argument("suffix-bytes must be 1 or 2");
    }
    return options;
}

unsigned hex_value(char ch) {
    if (ch >= '0' && ch <= '9') return static_cast<unsigned>(ch - '0');
    if (ch >= 'a' && ch <= 'f') return 10U + static_cast<unsigned>(ch - 'a');
    if (ch >= 'A' && ch <= 'F') return 10U + static_cast<unsigned>(ch - 'A');
    throw std::invalid_argument("invalid hexadecimal message");
}

std::vector<std::uint8_t> parse_hex(std::string_view text) {
    if ((text.size() & 1U) != 0U) {
        throw std::invalid_argument("hexadecimal message must have even length");
    }
    std::vector<std::uint8_t> result(text.size() / 2U);
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<std::uint8_t>(
            (hex_value(text[i * 2U]) << 4U) | hex_value(text[i * 2U + 1U]));
    }
    return result;
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

std::vector<std::uint8_t> joined(const std::vector<std::uint8_t>& prefix,
                                 std::uint64_t suffix_counter,
                                 std::size_t suffix_bytes) {
    auto message = prefix;
    const auto suffix = suffix_from_counter(suffix_counter, suffix_bytes);
    message.insert(message.end(), suffix.begin(), suffix.end());
    return message;
}

struct CandidateBucket {
    std::vector<std::uint32_t> suffixes;
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto left_prefix = parse_hex(options.left_hex);
        const auto right_prefix = parse_hex(options.right_hex);
        if (left_prefix.size() != right_prefix.size()) {
            throw std::invalid_argument("prefixes must have equal length");
        }
        const auto left_forward = pvc::forward_state_for_research(
            left_prefix, preset.parameters);
        const auto right_forward = pvc::forward_state_for_research(
            right_prefix, preset.parameters);
        if (left_forward != right_forward) {
            throw std::invalid_argument("prefixes must be an exact forward-state collision");
        }

        const std::uint64_t full_domain = 1ULL
            << static_cast<unsigned>(options.suffix_bytes * 8U);
        const auto domain = std::min(full_domain, options.limit);

        std::cout << "Constrained independent-suffix foldback merge search\n"
                  << "preset=" << preset.name
                  << " left=" << options.left_hex
                  << " right=" << options.right_hex
                  << " suffix_bytes=" << options.suffix_bytes
                  << " domain_per_side=" << domain
                  << " cross_pair_space=" << domain * domain << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::unordered_map<pvc::tool::StateFingerprint,
                           CandidateBucket,
                           pvc::tool::StateFingerprintHash> left_table;
        left_table.reserve(static_cast<std::size_t>(domain * 2U));

        for (std::uint64_t suffix = 0; suffix < domain; ++suffix) {
            const auto message = joined(left_prefix, suffix, options.suffix_bytes);
            const auto state = pvc::foldback_state_for_research(
                message, preset.parameters);
            left_table[pvc::tool::analysis_fingerprint(state)]
                .suffixes.push_back(static_cast<std::uint32_t>(suffix));
        }

        std::uint64_t fingerprint_matches = 0;
        std::uint64_t exact_merges = 0;
        std::uint64_t equal_suffix_merges = 0;
        std::size_t printed = 0;

        for (std::uint64_t right_suffix = 0; right_suffix < domain; ++right_suffix) {
            const auto right_message = joined(
                right_prefix, right_suffix, options.suffix_bytes);
            const auto right_state = pvc::foldback_state_for_research(
                right_message, preset.parameters);
            const auto fingerprint = pvc::tool::analysis_fingerprint(right_state);
            const auto found = left_table.find(fingerprint);
            if (found == left_table.end()) {
                continue;
            }

            fingerprint_matches += found->second.suffixes.size();
            for (const auto left_suffix : found->second.suffixes) {
                const auto left_message = joined(
                    left_prefix, left_suffix, options.suffix_bytes);
                const auto left_state = pvc::foldback_state_for_research(
                    left_message, preset.parameters);
                if (left_state != right_state) {
                    continue;
                }
                ++exact_merges;
                if (left_suffix == right_suffix) {
                    ++equal_suffix_merges;
                }
                if (printed < options.print_limit) {
                    std::cout << "exact_foldback_merge left_suffix="
                              << pvc::tool::hex_bytes(suffix_from_counter(
                                  left_suffix, options.suffix_bytes))
                              << " right_suffix="
                              << pvc::tool::hex_bytes(suffix_from_counter(
                                  right_suffix, options.suffix_bytes))
                              << " left_message=" << pvc::tool::hex_bytes(left_message)
                              << " right_message=" << pvc::tool::hex_bytes(right_message)
                              << '\n';
                    ++printed;
                }
            }
        }

        std::cout << "fingerprint_candidate_pairs=" << fingerprint_matches << '\n'
                  << "exact_after_foldback_merges=" << exact_merges << '\n'
                  << "equal_suffix_merges=" << equal_suffix_merges << '\n'
                  << "different_suffix_merges="
                  << (exact_merges - equal_suffix_merges) << '\n';
        if (exact_merges == 0U) {
            std::cout << "No equal-length after-foldback state merge found; "
                         "therefore no full hash collision exists in this searched cross-domain.\n";
        }
        return exact_merges == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
