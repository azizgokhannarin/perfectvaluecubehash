#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    unsigned delta = 42;
    std::size_t print_limit = 16;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--delta" && i + 1 < argc) {
            options.delta = static_cast<unsigned>(std::stoul(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-alignment-probe [--preset NAME] [--delta 1..255] "
                "[--print-limit N]");
        }
    }
    if (options.delta == 0U || options.delta > 255U) {
        throw std::invalid_argument("delta must be in [1,255]");
    }
    return options;
}

bool same_physical_move(const pvc::Move& left, const pvc::Move& right) {
    return left.axis == right.axis
        && left.intersection_before == right.intersection_before
        && left.intersection_after == right.intersection_after
        && left.amount == right.amount
        && left.phase == right.phase
        && left.symbol_index == right.symbol_index;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        std::vector<std::uint64_t> prefix_histogram(
            preset.parameters.moves_per_symbol + 1U, 0U);
        std::uint64_t tested = 0;
        std::uint64_t at_least_one = 0;
        std::uint64_t exact_states = 0;
        std::size_t printed = 0;

        const auto initial = pvc::initial_internal_state();
        for (unsigned first = 0; first < 256U; ++first) {
            const auto context = pvc::absorb_symbol_for_research(
                initial, static_cast<std::uint8_t>(first), preset.parameters);
            for (unsigned symbol = 0; symbol + options.delta < 256U; ++symbol) {
                std::vector<pvc::Move> left_trace;
                std::vector<pvc::Move> right_trace;
                const auto left = pvc::absorb_symbol_for_research(
                    context, static_cast<std::uint8_t>(symbol),
                    preset.parameters, &left_trace);
                const auto right = pvc::absorb_symbol_for_research(
                    context, static_cast<std::uint8_t>(symbol + options.delta),
                    preset.parameters, &right_trace);
                ++tested;

                std::size_t equal_prefix = 0;
                while (equal_prefix < left_trace.size()
                       && equal_prefix < right_trace.size()
                       && same_physical_move(left_trace[equal_prefix],
                                             right_trace[equal_prefix])) {
                    ++equal_prefix;
                }
                ++prefix_histogram[equal_prefix];
                if (equal_prefix > 0U) {
                    ++at_least_one;
                }
                if (left == right) {
                    ++exact_states;
                    if (printed < options.print_limit) {
                        std::cout << "exact_alignment context="
                                  << pvc::tool::sequence_hex(first, 1U)
                                  << " symbols="
                                  << pvc::tool::sequence_hex(symbol, 1U)
                                  << '/' << pvc::tool::sequence_hex(
                                      symbol + options.delta, 1U)
                                  << " equal_moves=" << equal_prefix << '/'
                                  << preset.parameters.moves_per_symbol << '\n';
                        ++printed;
                    }
                }
            }
        }

        std::cout << "Alignment probe\n"
                  << "preset=" << preset.name << " delta=" << options.delta << '\n';
        pvc::tool::print_parameters(preset.parameters);
        std::cout << "tested_pairs=" << tested << '\n'
                  << "pairs_with_at_least_one_equal_move=" << at_least_one << '\n'
                  << "exact_transition_convergences=" << exact_states << '\n'
                  << "equal_move_prefix,hits\n";
        for (std::size_t i = 0; i < prefix_histogram.size(); ++i) {
            std::cout << i << ',' << prefix_histogram[i] << '\n';
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
