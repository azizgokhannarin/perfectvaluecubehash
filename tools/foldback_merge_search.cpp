#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
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
    std::string left_hex = "176f";
    std::string right_hex = "1799";
    std::size_t suffix_bytes = 2;
    std::uint64_t limit = 65536;
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
        } else {
            throw std::invalid_argument(
                "usage: pvc-foldback-merge-search [--preset NAME] "
                "[--left HEX] [--right HEX] [--suffix-bytes 0..4] [--limit N]");
        }
    }
    if (options.suffix_bytes > 4U) {
        throw std::invalid_argument("suffix-bytes must be at most 4");
    }
    return options;
}

unsigned hex_value(char ch) {
    if (ch >= '0' && ch <= '9') {
        return static_cast<unsigned>(ch - '0');
    }
    if (ch >= 'a' && ch <= 'f') {
        return 10U + static_cast<unsigned>(ch - 'a');
    }
    if (ch >= 'A' && ch <= 'F') {
        return 10U + static_cast<unsigned>(ch - 'A');
    }
    throw std::invalid_argument("invalid hexadecimal message");
}

std::vector<std::uint8_t> parse_hex(std::string_view text) {
    if ((text.size() & 1U) != 0U) {
        throw std::invalid_argument("hexadecimal message must have even length");
    }
    std::vector<std::uint8_t> bytes(text.size() / 2U);
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<std::uint8_t>(
            (hex_value(text[i * 2U]) << 4U) | hex_value(text[i * 2U + 1U]));
    }
    return bytes;
}

std::vector<std::uint8_t> suffix_from_counter(std::uint64_t counter,
                                              std::size_t bytes) {
    std::vector<std::uint8_t> suffix(bytes);
    for (std::size_t i = 0; i < bytes; ++i) {
        suffix[i] = static_cast<std::uint8_t>(counter >> (i * 8U));
    }
    return suffix;
}

const pvc::InternalStateSnapshot& phase_state(const pvc::ResearchHashResult& result,
                                              pvc::ResearchPhase phase) {
    if (phase == pvc::ResearchPhase::Final) {
        return result.final_state;
    }
    for (const auto& checkpoint : result.checkpoints) {
        if (checkpoint.phase == phase) {
            return checkpoint.state;
        }
    }
    throw std::logic_error("phase checkpoint missing");
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto left_prefix = parse_hex(options.left_hex);
        const auto right_prefix = parse_hex(options.right_hex);
        if (left_prefix.size() != right_prefix.size()) {
            throw std::invalid_argument("left and right prefixes must have equal length");
        }

        const auto left_forward = pvc::forward_state_for_research(
            left_prefix, preset.parameters);
        const auto right_forward = pvc::forward_state_for_research(
            right_prefix, preset.parameters);
        if (left_forward != right_forward) {
            throw std::invalid_argument(
                "supplied prefixes do not form an exact forward-state collision");
        }

        const std::uint64_t full_domain = options.suffix_bytes == 0U
                                        ? 1U
                                        : (1ULL << static_cast<unsigned>(options.suffix_bytes * 8U));
        const auto limit = std::min(options.limit, full_domain);

        std::cout << "Foldback merge-extension search\n"
                  << "preset=" << preset.name
                  << " left=" << options.left_hex
                  << " right=" << options.right_hex
                  << " suffix_bytes=" << options.suffix_bytes
                  << " limit=" << limit << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::size_t minimum_bits = std::numeric_limits<std::size_t>::max();
        std::size_t minimum_bytes = std::numeric_limits<std::size_t>::max();
        std::uint64_t minimum_counter = 0;

        for (std::uint64_t counter = 0; counter < limit; ++counter) {
            const auto suffix = suffix_from_counter(counter, options.suffix_bytes);
            auto left = left_prefix;
            auto right = right_prefix;
            left.insert(left.end(), suffix.begin(), suffix.end());
            right.insert(right.end(), suffix.begin(), suffix.end());

            const auto result_left = pvc::inspect_with_parameters(
                left, preset.parameters, false, true);
            const auto result_right = pvc::inspect_with_parameters(
                right, preset.parameters, false, true);

            const auto& fold_left = phase_state(
                result_left, pvc::ResearchPhase::AfterFoldback);
            const auto& fold_right = phase_state(
                result_right, pvc::ResearchPhase::AfterFoldback);
            const auto bit_distance = pvc::tool::cube_bit_distance(
                fold_left.cube, fold_right.cube);
            const auto byte_distance = pvc::tool::cube_byte_distance(
                fold_left.cube, fold_right.cube);

            if (bit_distance < minimum_bits
                || (bit_distance == minimum_bits && byte_distance < minimum_bytes)) {
                minimum_bits = bit_distance;
                minimum_bytes = byte_distance;
                minimum_counter = counter;
            }

            if (fold_left == fold_right) {
                std::cout << "after-foldback collision found\n"
                          << "suffix=" << pvc::tool::hex_bytes(suffix) << '\n'
                          << "left_message=" << pvc::tool::hex_bytes(left) << '\n'
                          << "right_message=" << pvc::tool::hex_bytes(right) << '\n'
                          << "final_state_equal="
                          << (result_left.final_state == result_right.final_state
                              ? "yes" : "no") << '\n'
                          << "digest_equal="
                          << (result_left.digest == result_right.digest ? "yes" : "no")
                          << '\n';
                return result_left.digest == result_right.digest ? 1 : 0;
            }
        }

        const auto minimum_suffix = suffix_from_counter(
            minimum_counter, options.suffix_bytes);
        std::cout << "No after-foldback collision found.\n"
                  << "minimum_foldback_cube_bit_distance=" << minimum_bits << '\n'
                  << "minimum_foldback_cube_byte_distance=" << minimum_bytes << '\n'
                  << "minimum_suffix=" << pvc::tool::hex_bytes(minimum_suffix) << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
