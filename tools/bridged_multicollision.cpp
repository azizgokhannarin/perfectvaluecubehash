#include "alias_catalog.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t levels = 16U;
    std::size_t materialize_levels = 16U;
    std::size_t prefix_count = 65536U;
    std::size_t threads = 0U;
    std::size_t print_limit = 16U;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--levels" && i + 1 < argc) {
            options.levels = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--materialize-levels" && i + 1 < argc) {
            options.materialize_levels = static_cast<std::size_t>(
                std::stoull(argv[++i]));
        } else if (arg == "--prefix-count" && i + 1 < argc) {
            options.prefix_count = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            options.threads = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-bridged-multicollision [--preset NAME] "
                "[--levels 2..32] [--materialize-levels 0..20] "
                "[--prefix-count 1..65536] [--threads N] [--print-limit N]");
        }
    }
    if (options.levels < 2U || options.levels > 32U) {
        throw std::invalid_argument("levels must be in [2,32]");
    }
    if (options.materialize_levels > 20U) {
        throw std::invalid_argument("materialize-levels must be in [0,20]");
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    return options;
}

struct BridgedAliasStep {
    std::optional<std::uint8_t> bridge;
    pvc::tool::SymbolAlias alias{};
};

std::optional<std::vector<BridgedAliasStep>> find_path(
    const pvc::InternalStateSnapshot& state,
    const pvc::HashParameters& parameters,
    std::size_t remaining_levels) {
    if (remaining_levels == 0U) {
        return std::vector<BridgedAliasStep>{};
    }

    const auto direct_aliases = pvc::tool::find_symbol_aliases(state, parameters);
    for (const auto& alias : direct_aliases) {
        auto tail = find_path(alias.common_state, parameters, remaining_levels - 1U);
        if (tail.has_value()) {
            tail->insert(tail->begin(), BridgedAliasStep{
                .bridge = std::nullopt,
                .alias = alias,
            });
            return tail;
        }
    }

    for (std::uint16_t bridge = 0U; bridge < 256U; ++bridge) {
        const auto bridged_state = pvc::absorb_symbol_for_research(
            state, static_cast<std::uint8_t>(bridge), parameters);
        const auto aliases = pvc::tool::find_symbol_aliases(
            bridged_state, parameters);
        for (const auto& alias : aliases) {
            auto tail = find_path(
                alias.common_state, parameters, remaining_levels - 1U);
            if (tail.has_value()) {
                tail->insert(tail->begin(), BridgedAliasStep{
                    .bridge = static_cast<std::uint8_t>(bridge),
                    .alias = alias,
                });
                return tail;
            }
        }
    }
    return std::nullopt;
}

struct FoundPath {
    pvc::tool::ForwardCollisionPair seed{};
    std::vector<BridgedAliasStep> steps;
};

std::optional<FoundPath> find_seeded_path(
    const std::vector<pvc::tool::ForwardCollisionPair>& seeds,
    const pvc::HashParameters& parameters,
    std::size_t levels) {
    for (const auto& seed : seeds) {
        auto steps = find_path(seed.common_state, parameters, levels - 1U);
        if (steps.has_value()) {
            return FoundPath{.seed = seed, .steps = std::move(*steps)};
        }
    }
    return std::nullopt;
}

std::vector<std::vector<std::uint8_t>> materialize_messages(
    const FoundPath& path,
    std::size_t levels) {
    std::vector<std::vector<std::uint8_t>> messages{
        pvc::tool::as_vector(path.seed.left),
        pvc::tool::as_vector(path.seed.right),
    };
    for (std::size_t index = 0U; index + 1U < levels; ++index) {
        const auto& step = path.steps[index];
        if (step.bridge.has_value()) {
            for (auto& message : messages) {
                message.push_back(*step.bridge);
            }
        }
        std::vector<std::vector<std::uint8_t>> expanded;
        expanded.reserve(messages.size() * 2U);
        for (const auto& message : messages) {
            auto left = message;
            auto right = message;
            left.push_back(step.alias.left);
            right.push_back(step.alias.right);
            expanded.push_back(std::move(left));
            expanded.push_back(std::move(right));
        }
        messages = std::move(expanded);
    }
    return messages;
}

struct ExactStateBucket {
    std::vector<std::size_t> indices;
};

struct MaterializedResult {
    std::size_t messages = 0U;
    std::size_t length = 0U;
    std::size_t forward_mismatches = 0U;
    std::size_t foldback_collisions = 0U;
    std::size_t digest_collisions = 0U;
    std::size_t unique_foldback_states = 0U;
    std::size_t unique_digests = 0U;
};

MaterializedResult evaluate_messages(
    const std::vector<std::vector<std::uint8_t>>& messages,
    const pvc::InternalStateSnapshot& common_forward,
    const pvc::HashParameters& parameters) {
    MaterializedResult result{
        .messages = messages.size(),
        .length = messages.empty() ? 0U : messages.front().size(),
    };
    std::vector<pvc::InternalStateSnapshot> foldback_states;
    foldback_states.reserve(messages.size());
    std::unordered_map<pvc::tool::StateFingerprint,
                       ExactStateBucket,
                       pvc::tool::StateFingerprintHash> state_table;
    state_table.reserve(messages.size() * 2U);
    std::unordered_map<std::string, std::size_t> digest_table;
    digest_table.reserve(messages.size() * 2U);

    for (std::size_t index = 0U; index < messages.size(); ++index) {
        if (pvc::forward_state_for_research(messages[index], parameters)
            != common_forward) {
            ++result.forward_mismatches;
        }
        auto state = pvc::foldback_state_for_research(messages[index], parameters);
        const auto fingerprint = pvc::tool::analysis_fingerprint(state);
        auto& bucket = state_table[fingerprint].indices;
        bool exact_state_collision = false;
        for (const auto prior : bucket) {
            if (foldback_states[prior] == state) {
                exact_state_collision = true;
                break;
            }
        }
        if (exact_state_collision) {
            ++result.foldback_collisions;
        }
        bucket.push_back(index);
        foldback_states.push_back(std::move(state));

        const auto digest = pvc::hash_with_parameters(messages[index], parameters);
        const std::string digest_key(
            reinterpret_cast<const char*>(digest.data()), digest.size());
        if (!digest_table.emplace(digest_key, index).second) {
            ++result.digest_collisions;
        }
    }
    result.unique_foldback_states = messages.size() - result.foldback_collisions;
    result.unique_digests = digest_table.size();
    return result;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto seeds = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);

        std::cout << "Bridged forward multicollision construction\n"
                  << "preset=" << preset.name
                  << " requested_levels=" << options.levels
                  << " theoretical_messages=2^" << options.levels
                  << " seed_pairs=" << seeds.size() << '\n';
        pvc::tool::print_parameters(preset.parameters);

        const auto found = find_seeded_path(
            seeds, preset.parameters, options.levels);
        if (!found.has_value()) {
            std::cout << "path_found=no\n";
            return 1;
        }

        std::cout << "path_found=yes\n"
                  << "seed="
                  << pvc::tool::hex_bytes(pvc::tool::as_vector(found->seed.left))
                  << '/'
                  << pvc::tool::hex_bytes(pvc::tool::as_vector(found->seed.right))
                  << '\n';
        std::size_t bridge_steps = 0U;
        for (std::size_t i = 0U; i < found->steps.size(); ++i) {
            const auto& step = found->steps[i];
            if (step.bridge.has_value()) {
                ++bridge_steps;
            }
            if (i < options.print_limit) {
                std::cout << "level=" << (i + 2U)
                          << " bridge=";
                if (step.bridge.has_value()) {
                    std::cout << pvc::tool::sequence_hex(*step.bridge, 1U);
                } else {
                    std::cout << "none";
                }
                std::cout << " alias="
                          << pvc::tool::sequence_hex(step.alias.left, 1U)
                          << '/'
                          << pvc::tool::sequence_hex(step.alias.right, 1U)
                          << '\n';
            }
        }
        std::cout << "bridge_steps=" << bridge_steps << '\n';

        const auto materialized_levels = std::min(
            options.materialize_levels, options.levels);
        if (materialized_levels == 0U) {
            std::cout << "materialized_levels=0\n";
            return 0;
        }
        const auto messages = materialize_messages(*found, materialized_levels);
        auto common_state = found->seed.common_state;
        for (std::size_t i = 0U; i + 1U < materialized_levels; ++i) {
            const auto& step = found->steps[i];
            if (step.bridge.has_value()) {
                common_state = pvc::absorb_symbol_for_research(
                    common_state, *step.bridge, preset.parameters);
            }
            common_state = step.alias.common_state;
        }
        const auto result = evaluate_messages(
            messages, common_state, preset.parameters);
        std::cout << "materialized_levels=" << materialized_levels << '\n'
                  << "materialized_messages=" << result.messages << '\n'
                  << "materialized_message_length=" << result.length << '\n'
                  << "forward_state_mismatches=" << result.forward_mismatches << '\n'
                  << "unique_after_foldback_states="
                  << result.unique_foldback_states << '\n'
                  << "after_foldback_collisions="
                  << result.foldback_collisions << '\n'
                  << "unique_full_digests=" << result.unique_digests << '\n'
                  << "full_digest_collisions=" << result.digest_collisions << '\n';
        return result.forward_mismatches == 0U
                    && result.foldback_collisions == 0U
                    && result.digest_collisions == 0U
            ? 0
            : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
