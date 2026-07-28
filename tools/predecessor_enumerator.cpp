#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R2-small";
    std::size_t prefixes = 256;
    std::size_t prefix_bytes = 1;
    std::size_t symbols = 256;
};

struct Entry {
    std::size_t prefix = 0;
    std::uint8_t symbol = 0;
    std::size_t count = 1;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--prefixes" && i + 1 < argc) {
            options.prefixes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--prefix-bytes" && i + 1 < argc) {
            options.prefix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--symbols" && i + 1 < argc) {
            options.symbols = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-predecessor-enumerator [--preset NAME] "
                "[--prefixes N] [--prefix-bytes 1..8] [--symbols 1..256]");
        }
    }
    if (options.prefix_bytes == 0U || options.prefix_bytes > 8U) {
        throw std::invalid_argument("prefix-bytes must be in [1,8]");
    }
    if (options.symbols == 0U || options.symbols > 256U) {
        throw std::invalid_argument("symbols must be in [1,256]");
    }
    return options;
}

std::vector<std::uint8_t> prefix_message(std::size_t value, std::size_t bytes) {
    std::vector<std::uint8_t> message(bytes);
    for (std::size_t i = 0; i < bytes; ++i) {
        message[i] = static_cast<std::uint8_t>(value >> (i * 8U));
    }
    return message;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);

        std::cout << "Reachable predecessor enumeration\n"
                  << "preset=" << preset.name
                  << " prefixes=" << options.prefixes
                  << " prefix_bytes=" << options.prefix_bytes
                  << " symbols=" << options.symbols << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::map<std::vector<std::uint8_t>, Entry, pvc::tool::StateKeyLess> targets;
        std::size_t collision_targets = 0;
        std::size_t extra_predecessors = 0;
        std::size_t max_indegree = 1;
        bool printed_first = false;

        for (std::size_t prefix = 0; prefix < options.prefixes; ++prefix) {
            const auto message = prefix_message(prefix, options.prefix_bytes);
            const auto before = pvc::forward_state_for_research(message, preset.parameters);
            for (std::size_t symbol_value = 0;
                 symbol_value < options.symbols;
                 ++symbol_value) {
                const auto symbol = static_cast<std::uint8_t>(symbol_value);
                const auto after = pvc::absorb_symbol_for_research(
                    before, symbol, preset.parameters);
                const auto key = pvc::tool::serialize_state(after);
                const auto [it, inserted] = targets.emplace(
                    key, Entry{.prefix = prefix, .symbol = symbol, .count = 1U});
                if (!inserted) {
                    if (it->second.count == 1U) {
                        ++collision_targets;
                    }
                    ++it->second.count;
                    ++extra_predecessors;
                    max_indegree = std::max(max_indegree, it->second.count);
                    if (!printed_first) {
                        std::cout << "first exact reachable-state merge\n"
                                  << "a_prefix=" << it->second.prefix
                                  << " a_symbol=" << static_cast<unsigned>(it->second.symbol)
                                  << '\n'
                                  << "b_prefix=" << prefix
                                  << " b_symbol=" << symbol_value << '\n';
                        printed_first = true;
                    }
                }
            }
        }

        const auto transitions = options.prefixes * options.symbols;
        std::cout << "transitions=" << transitions << '\n'
                  << "distinct_target_states=" << targets.size() << '\n'
                  << "targets_with_multiple_predecessors=" << collision_targets << '\n'
                  << "extra_predecessors=" << extra_predecessors << '\n'
                  << "maximum_observed_indegree=" << max_indegree << '\n';
        return extra_predecessors == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
