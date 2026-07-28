#include "bridged_path.hpp"
#include "digest_search_common.hpp"
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
#include <unordered_set>
#include <utility>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t levels = 16U;
    std::size_t beam_width = 256U;
    std::size_t prefix_count = 65536U;
    std::size_t threads = 0U;
    std::size_t print_limit = 8U;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--levels" && i + 1 < argc) {
            options.levels = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--beam" && i + 1 < argc) {
            options.beam_width = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--prefix-count" && i + 1 < argc) {
            options.prefix_count = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            options.threads = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-digest-beam-search [--preset NAME] "
                "[--levels 2..32] [--beam N] [--prefix-count 1..65536] "
                "[--threads N] [--print-limit N]");
        }
    }
    if (options.levels < 2U || options.levels > 32U) {
        throw std::invalid_argument("levels must be in [2,32]");
    }
    if (options.beam_width == 0U || options.beam_width > 65536U) {
        throw std::invalid_argument("beam must be in [1,65536]");
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    return options;
}

struct Candidate {
    std::vector<std::uint8_t> left;
    std::vector<std::uint8_t> right;
    std::vector<std::uint8_t> left_digest;
    std::vector<std::uint8_t> right_digest;
    std::size_t digest_bits = std::numeric_limits<std::size_t>::max();
    std::size_t foldback_bits = std::numeric_limits<std::size_t>::max();
};

std::string pair_key(std::vector<std::uint8_t>& left,
                     std::vector<std::uint8_t>& right) {
    if (right < left) {
        std::swap(left, right);
    }
    std::string key;
    key.reserve(left.size() + right.size() + 1U);
    key.append(reinterpret_cast<const char*>(left.data()), left.size());
    key.push_back('\0');
    key.append(reinterpret_cast<const char*>(right.data()), right.size());
    return key;
}

Candidate score_candidate(std::vector<std::uint8_t> left,
                          std::vector<std::uint8_t> right,
                          const pvc::InternalStateSnapshot& common_forward,
                          const pvc::HashParameters& parameters) {
    auto left_digest = pvc::hash_with_parameters(left, parameters);
    auto right_digest = pvc::hash_with_parameters(right, parameters);
    const auto left_foldback = pvc::foldback_from_forward_state_for_research(
        common_forward, left, parameters);
    const auto right_foldback = pvc::foldback_from_forward_state_for_research(
        common_forward, right, parameters);
    const auto digest_bits = pvc::tool::byte_hamming_distance(
        left_digest, right_digest);
    const auto foldback_bits = pvc::tool::operational_state_bit_distance(
        left_foldback, right_foldback);
    return Candidate{
        .left = std::move(left),
        .right = std::move(right),
        .left_digest = std::move(left_digest),
        .right_digest = std::move(right_digest),
        .digest_bits = digest_bits,
        .foldback_bits = foldback_bits,
    };
}

bool candidate_less(const Candidate& left, const Candidate& right) {
    if (left.digest_bits != right.digest_bits) {
        return left.digest_bits < right.digest_bits;
    }
    if (left.foldback_bits != right.foldback_bits) {
        return left.foldback_bits < right.foldback_bits;
    }
    if (left.left != right.left) {
        return left.left < right.left;
    }
    return left.right < right.right;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto seeds = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);
        const auto path = pvc::tool::find_bridged_path(
            seeds, preset.parameters, options.levels);
        if (!path.has_value()) {
            std::cout << "path_found=no\n";
            return 1;
        }

        std::cout << "Digest-guided beam search over a bridged forward "
                     "multicollision path\n"
                  << "preset=" << preset.name
                  << " levels=" << options.levels
                  << " beam_width=" << options.beam_width
                  << " seed_pairs=" << seeds.size() << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<Candidate> beam;
        beam.push_back(score_candidate(
            pvc::tool::as_vector(path->seed.left),
            pvc::tool::as_vector(path->seed.right),
            path->seed.common_state,
            preset.parameters));
        auto common_forward = path->seed.common_state;
        Candidate global_best = beam.front();
        std::size_t global_best_level = 1U;
        std::uint64_t cumulative_evaluated = 1U;

        std::cout << "level=1 candidates=1 best_digest_bits="
                  << beam.front().digest_bits
                  << " best_foldback_bits=" << beam.front().foldback_bits << '\n';

        for (std::size_t level = 2U; level <= options.levels; ++level) {
            const auto& step = path->steps[level - 2U];
            if (step.bridge.has_value()) {
                common_forward = pvc::absorb_symbol_for_research(
                    common_forward, *step.bridge, preset.parameters);
            }
            common_forward = step.alias.common_state;

            std::vector<Candidate> expanded;
            expanded.reserve(beam.size() * 4U);
            std::unordered_set<std::string> seen;
            seen.reserve(beam.size() * 8U);
            const std::array<std::uint8_t, 2> symbols{
                step.alias.left, step.alias.right};

            for (const auto& candidate : beam) {
                for (const auto left_symbol : symbols) {
                    for (const auto right_symbol : symbols) {
                        auto left = candidate.left;
                        auto right = candidate.right;
                        if (step.bridge.has_value()) {
                            left.push_back(*step.bridge);
                            right.push_back(*step.bridge);
                        }
                        left.push_back(left_symbol);
                        right.push_back(right_symbol);
                        if (left == right) {
                            continue;
                        }
                        auto key_left = left;
                        auto key_right = right;
                        const auto key = pair_key(key_left, key_right);
                        if (!seen.insert(key).second) {
                            continue;
                        }
                        expanded.push_back(score_candidate(
                            std::move(key_left), std::move(key_right),
                            common_forward, preset.parameters));
                    }
                }
            }

            const auto comparisons = static_cast<std::uint64_t>(expanded.size());
            cumulative_evaluated += comparisons;
            std::sort(expanded.begin(), expanded.end(), candidate_less);
            if (expanded.size() > options.beam_width) {
                expanded.resize(options.beam_width);
            }
            beam = std::move(expanded);
            if (beam.empty()) {
                throw std::runtime_error("beam became empty");
            }
            if (candidate_less(beam.front(), global_best)) {
                global_best = beam.front();
                global_best_level = level;
            }
            std::cout << "level=" << level
                      << " candidates=" << beam.size()
                      << " expanded=" << comparisons
                      << " generic_level_min_digest_bits="
                      << pvc::tool::generic_minimum_distance(comparisons)
                      << " generic_cumulative_min_digest_bits="
                      << pvc::tool::generic_minimum_distance(cumulative_evaluated)
                      << " best_digest_bits=" << beam.front().digest_bits
                      << " best_foldback_bits=" << beam.front().foldback_bits
                      << '\n';
            if (beam.front().digest_bits == 0U) {
                break;
            }
        }

        const auto limit = std::min(options.print_limit, beam.size());
        for (std::size_t i = 0U; i < limit; ++i) {
            std::cout << "rank=" << (i + 1U)
                      << " digest_bits=" << beam[i].digest_bits
                      << " foldback_bits=" << beam[i].foldback_bits
                      << " left=" << pvc::tool::hex_bytes(beam[i].left)
                      << " right=" << pvc::tool::hex_bytes(beam[i].right)
                      << '\n';
        }
        std::cout << "cumulative_pairs_evaluated=" << cumulative_evaluated << '\n'
                  << "generic_cumulative_min_digest_bits="
                  << pvc::tool::generic_minimum_distance(cumulative_evaluated) << '\n'
                  << "global_best_level=" << global_best_level << '\n'
                  << "global_best_digest_bits=" << global_best.digest_bits << '\n'
                  << "global_best_foldback_bits=" << global_best.foldback_bits << '\n'
                  << "global_best_left=" << pvc::tool::hex_bytes(global_best.left) << '\n'
                  << "global_best_right=" << pvc::tool::hex_bytes(global_best.right) << '\n'
                  << "exact_digest_collision="
                  << (global_best.digest_bits == 0U ? "yes" : "no") << '\n';
        pvc::tool::print_phase_distance_profile(
            global_best.left, global_best.right, preset.parameters);
        return global_best.digest_bits == 0U ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
