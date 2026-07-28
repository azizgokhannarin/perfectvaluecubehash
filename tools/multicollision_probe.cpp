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
    std::size_t max_levels = 4U;
    std::size_t suffix_bytes = 1U;
    std::uint64_t suffix_limit = 256U;
    std::size_t print_limit = 16U;
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
        } else if (arg == "--max-levels" && i + 1 < argc) {
            options.max_levels = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-limit" && i + 1 < argc) {
            options.suffix_limit = std::stoull(argv[++i]);
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-multicollision-probe [--preset NAME] "
                "[--prefix-count 1..65536] [--threads N] "
                "[--max-levels 1..8] [--suffix-bytes 0..2] "
                "[--suffix-limit N] [--print-limit N]");
        }
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    if (options.max_levels == 0U || options.max_levels > 8U) {
        throw std::invalid_argument("max-levels must be in [1,8]");
    }
    if (options.suffix_bytes > 2U) {
        throw std::invalid_argument("suffix-bytes must be in [0,2]");
    }
    return options;
}

struct AliasPath {
    std::vector<pvc::tool::SymbolAlias> aliases;
};

struct BranchRecord {
    std::size_t seed_index = 0U;
    pvc::tool::SymbolAlias alias{};
};

AliasPath longest_path(const pvc::InternalStateSnapshot& state,
                       const pvc::HashParameters& parameters,
                       std::size_t remaining) {
    AliasPath best;
    if (remaining == 0U) {
        return best;
    }
    const auto aliases = pvc::tool::find_symbol_aliases(state, parameters);
    for (const auto& alias : aliases) {
        auto tail = longest_path(alias.common_state, parameters, remaining - 1U);
        tail.aliases.insert(tail.aliases.begin(), alias);
        if (tail.aliases.size() > best.aliases.size()) {
            best = std::move(tail);
        }
    }
    return best;
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

std::vector<std::vector<std::uint8_t>> branch_messages(
    const pvc::tool::ForwardCollisionPair& seed,
    const pvc::tool::SymbolAlias& alias,
    const std::vector<std::uint8_t>& suffix = {}) {
    std::vector<std::vector<std::uint8_t>> messages;
    messages.reserve(4U);
    for (const auto& prefix : {seed.left, seed.right}) {
        for (const auto symbol : {alias.left, alias.right}) {
            std::vector<std::uint8_t> message(prefix.begin(), prefix.end());
            message.push_back(symbol);
            message.insert(message.end(), suffix.begin(), suffix.end());
            messages.push_back(std::move(message));
        }
    }
    return messages;
}

std::size_t unique_foldback_states(
    const std::vector<std::vector<std::uint8_t>>& messages,
    const pvc::HashParameters& parameters,
    std::vector<pvc::InternalStateSnapshot>* output = nullptr) {
    std::vector<pvc::InternalStateSnapshot> states;
    for (const auto& message : messages) {
        const auto state = pvc::foldback_state_for_research(message, parameters);
        if (output != nullptr) {
            output->push_back(state);
        }
        if (std::find(states.begin(), states.end(), state) == states.end()) {
            states.push_back(state);
        }
    }
    return states.size();
}

std::size_t unique_digests(
    const std::vector<std::vector<std::uint8_t>>& messages,
    const pvc::HashParameters& parameters) {
    std::vector<std::vector<std::uint8_t>> digests;
    for (const auto& message : messages) {
        const auto digest = pvc::hash_with_parameters(message, parameters);
        if (std::find(digests.begin(), digests.end(), digest) == digests.end()) {
            digests.push_back(digest);
        }
    }
    return digests.size();
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const std::uint64_t full_suffix_domain = options.suffix_bytes == 0U
            ? std::uint64_t{1}
            : (std::uint64_t{1}
               << static_cast<unsigned>(options.suffix_bytes * 8U));
        const auto suffix_domain = std::min(full_suffix_domain, options.suffix_limit);

        std::cout << "Forward multicollision-chain probe\n"
                  << "preset=" << preset.name
                  << " prefix_count=" << options.prefix_count
                  << " max_levels=" << options.max_levels
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffix_domain=" << suffix_domain << '\n';
        pvc::tool::print_parameters(preset.parameters);

        const auto seeds = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);
        std::vector<BranchRecord> branches;
        std::array<std::uint64_t, 256> branch_delta_histogram{};
        std::size_t best_seed_index = 0U;
        AliasPath best_path;

        for (std::size_t i = 0; i < seeds.size(); ++i) {
            const auto aliases = pvc::tool::find_symbol_aliases(
                seeds[i].common_state, preset.parameters);
            for (const auto& alias : aliases) {
                branches.push_back(BranchRecord{.seed_index = i, .alias = alias});
                ++branch_delta_histogram[static_cast<std::uint8_t>(
                    alias.right - alias.left)];
            }
            if (options.max_levels > 1U) {
                auto path = longest_path(
                    seeds[i].common_state,
                    preset.parameters,
                    options.max_levels - 1U);
                if (path.aliases.size() > best_path.aliases.size()) {
                    best_seed_index = i;
                    best_path = std::move(path);
                }
            }
        }

        std::uint64_t extension_cases = 0U;
        std::uint64_t foldback_collision_cases = 0U;
        std::uint64_t digest_collision_cases = 0U;
        std::size_t minimum_pair_bits = std::numeric_limits<std::size_t>::max();
        std::vector<std::uint8_t> minimum_message_a;
        std::vector<std::uint8_t> minimum_message_b;

        for (const auto& branch : branches) {
            const auto& seed = seeds[branch.seed_index];
            for (std::uint64_t counter = 0U; counter < suffix_domain; ++counter) {
                const auto suffix = suffix_from_counter(counter, options.suffix_bytes);
                const auto messages = branch_messages(seed, branch.alias, suffix);
                std::vector<pvc::InternalStateSnapshot> states;
                const auto unique_states = unique_foldback_states(
                    messages, preset.parameters, &states);
                const auto unique_hashes = unique_digests(messages, preset.parameters);
                ++extension_cases;
                if (unique_states != messages.size()) {
                    ++foldback_collision_cases;
                }
                if (unique_hashes != messages.size()) {
                    ++digest_collision_cases;
                }
                for (std::size_t i = 0; i < states.size(); ++i) {
                    for (std::size_t j = i + 1U; j < states.size(); ++j) {
                        const auto distance = pvc::tool::cube_bit_distance(
                            states[i].cube, states[j].cube);
                        if (distance < minimum_pair_bits) {
                            minimum_pair_bits = distance;
                            minimum_message_a = messages[i];
                            minimum_message_b = messages[j];
                        }
                    }
                }
            }
        }

        std::cout << "three_byte_seed_pairs=" << seeds.size() << '\n'
                  << "seeds_with_next_symbol_alias=" << branches.size() << '\n'
                  << "next_symbol_alias_pairs=" << branches.size() << '\n'
                  << "maximum_collision_levels=" << (1U + best_path.aliases.size())
                  << '\n'
                  << "four_way_extension_cases=" << extension_cases << '\n'
                  << "extension_cases_with_foldback_collision="
                  << foldback_collision_cases << '\n'
                  << "extension_cases_with_digest_collision="
                  << digest_collision_cases << '\n'
                  << "minimum_foldback_pair_cube_bit_distance="
                  << minimum_pair_bits << '\n'
                  << "minimum_message_a="
                  << pvc::tool::hex_bytes(minimum_message_a) << '\n'
                  << "minimum_message_b="
                  << pvc::tool::hex_bytes(minimum_message_b) << '\n'
                  << "next_alias_delta,hits\n";
        for (std::size_t delta = 0U; delta < branch_delta_histogram.size(); ++delta) {
            if (branch_delta_histogram[delta] != 0U) {
                std::cout << delta << ',' << branch_delta_histogram[delta] << '\n';
            }
        }

        const auto records_to_print = std::min(options.print_limit, branches.size());
        for (std::size_t i = 0; i < records_to_print; ++i) {
            const auto& branch = branches[i];
            const auto& seed = seeds[branch.seed_index];
            std::cout << "branch[" << i << "] seed="
                      << pvc::tool::hex_bytes(pvc::tool::as_vector(seed.left))
                      << '/' << pvc::tool::hex_bytes(pvc::tool::as_vector(seed.right))
                      << " next=" << pvc::tool::sequence_hex(branch.alias.left, 1U)
                      << '/' << pvc::tool::sequence_hex(branch.alias.right, 1U)
                      << '\n';
        }

        if (!seeds.empty()) {
            const auto& seed = seeds[best_seed_index];
            std::vector<std::vector<std::uint8_t>> messages{
                pvc::tool::as_vector(seed.left),
                pvc::tool::as_vector(seed.right),
            };
            auto common_state = seed.common_state;
            for (const auto& alias : best_path.aliases) {
                std::vector<std::vector<std::uint8_t>> expanded;
                expanded.reserve(messages.size() * 2U);
                for (const auto& message : messages) {
                    auto left = message;
                    auto right = message;
                    left.push_back(alias.left);
                    right.push_back(alias.right);
                    expanded.push_back(std::move(left));
                    expanded.push_back(std::move(right));
                }
                messages = std::move(expanded);
                common_state = alias.common_state;
            }

            bool forward_equal = true;
            for (const auto& message : messages) {
                if (pvc::forward_state_for_research(message, preset.parameters)
                    != common_state) {
                    forward_equal = false;
                    break;
                }
            }
            std::cout << "constructed_messages=" << messages.size() << '\n'
                      << "constructed_length=" << messages.front().size() << '\n'
                      << "all_forward_states_equal="
                      << (forward_equal ? "yes" : "no") << '\n'
                      << "constructed_unique_after_foldback_states="
                      << unique_foldback_states(messages, preset.parameters) << '\n'
                      << "constructed_unique_full_digests="
                      << unique_digests(messages, preset.parameters) << '\n';
            const auto to_print = std::min(options.print_limit, messages.size());
            for (std::size_t i = 0; i < to_print; ++i) {
                std::cout << "message[" << i << "]="
                          << pvc::tool::hex_bytes(messages[i]) << '\n';
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
