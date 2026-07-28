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
    std::size_t depth = 2;
    std::uint64_t prefix_counter = 0;
    std::size_t prefix_bytes = 0;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--depth" && i + 1 < argc) {
            options.depth = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--prefix-counter" && i + 1 < argc) {
            options.prefix_counter = std::stoull(argv[++i]);
        } else if (arg == "--prefix-bytes" && i + 1 < argc) {
            options.prefix_bytes = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-transition-collision [--preset NAME] [--depth 1|2] "
                "[--prefix-counter N] [--prefix-bytes N]");
        }
    }
    if (options.depth == 0U || options.depth > 2U) {
        throw std::invalid_argument("depth must be 1 or 2");
    }
    if (options.prefix_bytes > 8U) {
        throw std::invalid_argument("prefix-bytes must be at most 8");
    }
    return options;
}

std::vector<std::uint8_t> make_prefix(std::uint64_t counter, std::size_t bytes) {
    std::vector<std::uint8_t> prefix(bytes);
    for (std::size_t i = 0; i < bytes; ++i) {
        prefix[i] = static_cast<std::uint8_t>(counter >> (i * 8U));
    }
    return prefix;
}

pvc::InternalStateSnapshot apply_sequence(const pvc::InternalStateSnapshot& start,
                                          std::uint32_t sequence,
                                          std::size_t depth,
                                          const pvc::HashParameters& parameters) {
    auto state = start;
    for (std::size_t i = 0; i < depth; ++i) {
        const auto shift = static_cast<unsigned>((depth - 1U - i) * 8U);
        const auto symbol = static_cast<std::uint8_t>((sequence >> shift) & 0xFFU);
        state = pvc::absorb_symbol_for_research(state, symbol, parameters);
    }
    return state;
}


std::vector<std::uint8_t> sequence_message(const std::vector<std::uint8_t>& prefix,
                                           std::uint32_t sequence,
                                           std::size_t depth) {
    auto message = prefix;
    message.reserve(prefix.size() + depth);
    for (std::size_t i = 0; i < depth; ++i) {
        const auto shift = static_cast<unsigned>((depth - 1U - i) * 8U);
        message.push_back(static_cast<std::uint8_t>((sequence >> shift) & 0xFFU));
    }
    return message;
}

const pvc::InternalStateSnapshot& find_phase(const pvc::ResearchHashResult& result,
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
        const auto prefix = make_prefix(options.prefix_counter, options.prefix_bytes);
        const auto start = pvc::forward_state_for_research(prefix, preset.parameters);
        const std::uint32_t sequence_count = options.depth == 1U ? 256U : 65536U;

        std::cout << "Transition collision search\n"
                  << "preset=" << preset.name << " depth=" << options.depth
                  << " prefix=" << pvc::tool::hex_bytes(prefix) << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::map<std::vector<std::uint8_t>, std::uint32_t, pvc::tool::StateKeyLess> seen;
        for (std::uint32_t sequence = 0; sequence < sequence_count; ++sequence) {
            const auto next = apply_sequence(start, sequence, options.depth, preset.parameters);
            const auto key = pvc::tool::serialize_state(next);
            const auto [it, inserted] = seen.emplace(key, sequence);
            if (!inserted) {
                const auto message_a = sequence_message(prefix, it->second, options.depth);
                const auto message_b = sequence_message(prefix, sequence, options.depth);
                const auto full_a = pvc::inspect_with_parameters(
                    message_a, preset.parameters, false, true);
                const auto full_b = pvc::inspect_with_parameters(
                    message_b, preset.parameters, false, true);

                std::cout << "exact transition collision found\n"
                          << "a=" << pvc::tool::sequence_hex(it->second, options.depth) << '\n'
                          << "b=" << pvc::tool::sequence_hex(sequence, options.depth) << '\n'
                          << "symbol_index=" << next.symbol_index << '\n'
                          << "after_foldback_equal="
                          << (find_phase(full_a, pvc::ResearchPhase::AfterFoldback)
                                  == find_phase(full_b, pvc::ResearchPhase::AfterFoldback)
                              ? "yes" : "no") << '\n'
                          << "after_orbit_equal="
                          << (find_phase(full_a, pvc::ResearchPhase::AfterOrbitClosure)
                                  == find_phase(full_b, pvc::ResearchPhase::AfterOrbitClosure)
                              ? "yes" : "no") << '\n'
                          << "final_state_equal="
                          << (full_a.final_state == full_b.final_state ? "yes" : "no") << '\n'
                          << "digest_equal="
                          << (full_a.digest == full_b.digest ? "yes" : "no") << '\n';
                return 1;
            }
        }

        std::cout << "No exact state collision among " << sequence_count
                  << " sequences from this start state.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
