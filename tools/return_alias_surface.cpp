#include "alias_catalog.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t messages = 512U;
    std::size_t message_bytes = 8U;
    std::size_t print_limit = 16U;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--messages" && i + 1 < argc) {
            options.messages = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--message-bytes" && i + 1 < argc) {
            options.message_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-return-alias-surface [--preset NAME] "
                "[--messages N] [--message-bytes 1..64] [--print-limit N]");
        }
    }
    if (options.messages == 0U || options.messages > 100000U) {
        throw std::invalid_argument("messages must be in [1,100000]");
    }
    if (options.message_bytes == 0U || options.message_bytes > 64U) {
        throw std::invalid_argument("message-bytes must be in [1,64]");
    }
    return options;
}

std::vector<std::uint8_t> sample_message(std::uint64_t counter,
                                         std::size_t bytes) {
    std::vector<std::uint8_t> message(bytes);
    std::uint64_t state = counter + 0x9e3779b97f4a7c15ULL;
    for (std::size_t i = 0U; i < bytes; ++i) {
        state ^= state >> 12U;
        state ^= state << 25U;
        state ^= state >> 27U;
        state *= 0x2545f4914f6cdd1dULL;
        message[i] = static_cast<std::uint8_t>(
            (state >> ((i & 7U) * 8U))
            ^ static_cast<std::uint64_t>(i * 37U));
    }
    return message;
}

struct DepthStats {
    std::uint64_t contexts = 0U;
    std::uint64_t contexts_with_alias = 0U;
    std::uint64_t alias_pairs = 0U;
    std::size_t maximum_aliases = 0U;
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        std::vector<DepthStats> per_depth(options.message_bytes + 1U);
        std::array<std::uint64_t, 256> delta_histogram{};
        std::map<std::size_t, std::uint64_t> aliases_per_context;
        std::size_t printed = 0U;

        std::cout << "Reachable foldback-controller alias surface\n"
                  << "preset=" << preset.name
                  << " messages=" << options.messages
                  << " message_bytes=" << options.message_bytes << '\n';
        pvc::tool::print_parameters(preset.parameters);

        for (std::size_t message_index = 0U;
             message_index < options.messages;
             ++message_index) {
            const auto message = sample_message(message_index, options.message_bytes);
            auto state = pvc::forward_state_for_research(message, preset.parameters);

            for (std::size_t reverse_depth = 0U;
                 reverse_depth <= message.size();
                 ++reverse_depth) {
                const auto aliases = pvc::tool::find_symbol_aliases(
                    state, preset.parameters);
                auto& depth = per_depth[reverse_depth];
                ++depth.contexts;
                depth.alias_pairs += aliases.size();
                depth.maximum_aliases = std::max(depth.maximum_aliases, aliases.size());
                ++aliases_per_context[aliases.size()];
                if (!aliases.empty()) {
                    ++depth.contexts_with_alias;
                }
                for (const auto& alias : aliases) {
                    const auto delta = static_cast<std::uint8_t>(
                        alias.left > alias.right
                            ? alias.left - alias.right
                            : alias.right - alias.left);
                    ++delta_histogram[delta];
                    if (printed < options.print_limit) {
                        std::cout << "sample_alias message=" << message_index
                                  << " reverse_depth=" << reverse_depth
                                  << " symbols="
                                  << pvc::tool::sequence_hex(alias.left, 1U)
                                  << '/'
                                  << pvc::tool::sequence_hex(alias.right, 1U)
                                  << " delta=" << static_cast<unsigned>(delta)
                                  << '\n';
                        ++printed;
                    }
                }

                if (reverse_depth == message.size()) {
                    break;
                }
                const auto original_index = message.size() - 1U - reverse_depth;
                const auto symbol = pvc::return_symbol_for_research(
                    message[original_index], original_index);
                state = pvc::absorb_symbol_for_research(
                    state, symbol, preset.parameters);
            }
        }

        std::uint64_t total_contexts = 0U;
        std::uint64_t total_alias_contexts = 0U;
        std::uint64_t total_aliases = 0U;
        std::size_t maximum_aliases = 0U;
        for (std::size_t depth = 0U; depth < per_depth.size(); ++depth) {
            const auto& stats = per_depth[depth];
            total_contexts += stats.contexts;
            total_alias_contexts += stats.contexts_with_alias;
            total_aliases += stats.alias_pairs;
            maximum_aliases = std::max(maximum_aliases, stats.maximum_aliases);
            std::cout << "reverse_depth=" << depth
                      << " contexts=" << stats.contexts
                      << " contexts_with_alias=" << stats.contexts_with_alias
                      << " alias_pairs=" << stats.alias_pairs
                      << " max_aliases_in_context=" << stats.maximum_aliases
                      << '\n';
        }

        std::cout << std::fixed << std::setprecision(8)
                  << "total_contexts=" << total_contexts << '\n'
                  << "contexts_with_alias=" << total_alias_contexts << '\n'
                  << "context_alias_probability="
                  << (static_cast<double>(total_alias_contexts)
                      / static_cast<double>(total_contexts)) << '\n'
                  << "total_alias_pairs=" << total_aliases << '\n'
                  << "mean_alias_pairs_per_context="
                  << (static_cast<double>(total_aliases)
                      / static_cast<double>(total_contexts)) << '\n'
                  << "maximum_alias_pairs_in_context=" << maximum_aliases << '\n'
                  << "alias_count,contexts\n";
        for (const auto& [count, contexts] : aliases_per_context) {
            if (count != 0U) {
                std::cout << count << ',' << contexts << '\n';
            }
        }
        std::cout << "delta,hits\n";
        for (std::size_t delta = 1U; delta < delta_histogram.size(); ++delta) {
            if (delta_histogram[delta] != 0U) {
                std::cout << delta << ',' << delta_histogram[delta] << '\n';
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
