#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t maximum_length = 64U;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--max-length" && i + 1 < argc) {
            options.maximum_length = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-length-framing-probe [--preset NAME] "
                "[--max-length N]");
        }
    }
    if (options.maximum_length > 100000U) {
        throw std::invalid_argument("max-length exceeds research limit 100000");
    }
    return options;
}

std::vector<std::uint8_t> message_for_length(std::size_t length) {
    std::vector<std::uint8_t> message(length);
    for (std::size_t i = 0U; i < length; ++i) {
        message[i] = static_cast<std::uint8_t>(i * 73U + length * 19U);
    }
    return message;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        std::unordered_set<std::uint64_t> forward_indices;
        std::unordered_set<std::uint64_t> foldback_indices;
        std::unordered_set<std::uint64_t> final_indices;
        std::size_t forward_mismatches = 0U;
        std::size_t foldback_mismatches = 0U;
        std::size_t final_mismatches = 0U;

        const std::uint64_t fixed_final_symbols =
            10U
            + static_cast<std::uint64_t>(preset.parameters.diagonal_closure_symbols)
            + static_cast<std::uint64_t>(preset.parameters.orbit_closure_symbols)
            + static_cast<std::uint64_t>(preset.parameters.squeeze_bytes)
                * static_cast<std::uint64_t>(
                    preset.parameters.squeeze_symbols_per_byte);

        std::cout << "Unequal-length operational-state framing probe\n"
                  << "preset=" << preset.name
                  << " maximum_length=" << options.maximum_length << '\n';
        pvc::tool::print_parameters(preset.parameters);

        for (std::size_t length = 0U; length <= options.maximum_length; ++length) {
            const auto message = message_for_length(length);
            const auto forward = pvc::forward_state_for_research(
                message, preset.parameters);
            const auto foldback = pvc::foldback_state_for_research(
                message, preset.parameters);
            const auto inspected = pvc::inspect_with_parameters(
                message, preset.parameters, false, false);
            const auto expected_forward = static_cast<std::uint64_t>(length);
            const auto expected_foldback = preset.parameters.enable_foldback
                ? expected_forward * 2U : expected_forward;
            const auto expected_final = expected_foldback + fixed_final_symbols;

            forward_mismatches += forward.symbol_index != expected_forward ? 1U : 0U;
            foldback_mismatches += foldback.symbol_index != expected_foldback ? 1U : 0U;
            final_mismatches += inspected.final_state.symbol_index != expected_final ? 1U : 0U;
            forward_indices.insert(forward.symbol_index);
            foldback_indices.insert(foldback.symbol_index);
            final_indices.insert(inspected.final_state.symbol_index);
        }

        const auto lengths = options.maximum_length + 1U;
        std::cout << "fixed_finalization_symbols=" << fixed_final_symbols << '\n'
                  << "tested_lengths=" << lengths << '\n'
                  << "unique_forward_symbol_indices=" << forward_indices.size() << '\n'
                  << "unique_foldback_symbol_indices=" << foldback_indices.size() << '\n'
                  << "unique_final_symbol_indices=" << final_indices.size() << '\n'
                  << "forward_formula_mismatches=" << forward_mismatches << '\n'
                  << "foldback_formula_mismatches=" << foldback_mismatches << '\n'
                  << "final_formula_mismatches=" << final_mismatches << '\n'
                  << "identical_full_state_across_unequal_lengths_blocked_by_index="
                  << ((forward_indices.size() == lengths
                       && foldback_indices.size() == lengths
                       && final_indices.size() == lengths)
                          ? "yes" : "no") << '\n'
                  << "note=This blocks classic identical-operational-state expandable "
                     "messages; it does not prove unequal-length digest collision resistance.\n";
        return forward_mismatches == 0U
                    && foldback_mismatches == 0U
                    && final_mismatches == 0U
            ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
