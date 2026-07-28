#include "bridged_path.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
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
                "usage: pvc-foldback-beam-search [--preset NAME] "
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
    std::size_t bit_distance = std::numeric_limits<std::size_t>::max();
    std::size_t byte_distance = std::numeric_limits<std::size_t>::max();
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
    const auto left_state = pvc::foldback_from_forward_state_for_research(
        common_forward, left, parameters);
    const auto right_state = pvc::foldback_from_forward_state_for_research(
        common_forward, right, parameters);
    return Candidate{
        .left = std::move(left),
        .right = std::move(right),
        .bit_distance = pvc::tool::operational_state_bit_distance(
            left_state, right_state),
        .byte_distance = pvc::tool::operational_state_byte_distance(
            left_state, right_state),
    };
}

bool candidate_less(const Candidate& left, const Candidate& right) {
    if (left.bit_distance != right.bit_distance) {
        return left.bit_distance < right.bit_distance;
    }
    if (left.byte_distance != right.byte_distance) {
        return left.byte_distance < right.byte_distance;
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

        std::cout << "Foldback-distance beam search over a bridged forward "
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

        std::cout << "level=1 candidates=1 best_bits=" << beam.front().bit_distance
                  << " best_bytes=" << beam.front().byte_distance << '\n';

        Candidate global_best = beam.front();
        std::size_t global_best_level = 1U;
        bool exact_collision = beam.front().bit_distance == 0U;
        for (std::size_t level = 2U;
             level <= options.levels && !exact_collision;
             ++level) {
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
            exact_collision = beam.front().bit_distance == 0U;
            std::cout << "level=" << level
                      << " candidates=" << beam.size()
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
                      << " best_bits=" << beam.front().bit_distance
                      << " best_bytes=" << beam.front().byte_distance << '\n';
        }

        const auto limit = std::min(options.print_limit, beam.size());
        for (std::size_t i = 0U; i < limit; ++i) {
            const auto left_digest = pvc::hash_with_parameters(
                beam[i].left, preset.parameters);
            const auto right_digest = pvc::hash_with_parameters(
                beam[i].right, preset.parameters);
            std::cout << "rank=" << (i + 1U)
                      << " foldback_bits=" << beam[i].bit_distance
                      << " foldback_bytes=" << beam[i].byte_distance
                      << " digest_bits=" << pvc::tool::byte_hamming_distance(
                            left_digest, right_digest)
                      << " left=" << pvc::tool::hex_bytes(beam[i].left)
                      << " right=" << pvc::tool::hex_bytes(beam[i].right)
                      << '\n';
        }
        const auto global_left_digest = pvc::hash_with_parameters(
            global_best.left, preset.parameters);
        const auto global_right_digest = pvc::hash_with_parameters(
            global_best.right, preset.parameters);
        std::cout << "global_best_level=" << global_best_level << '\n'
                  << "global_best_foldback_bits=" << global_best.bit_distance << '\n'
                  << "global_best_foldback_bytes=" << global_best.byte_distance << '\n'
                  << "global_best_digest_bits="
                  << pvc::tool::byte_hamming_distance(
                        global_left_digest, global_right_digest) << '\n'
                  << "global_best_left="
                  << pvc::tool::hex_bytes(global_best.left) << '\n'
                  << "global_best_right="
                  << pvc::tool::hex_bytes(global_best.right) << '\n'
                  << "exact_after_foldback_collision="
                  << (global_best.bit_distance == 0U ? "yes" : "no") << '\n';
        return global_best.bit_distance == 0U ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
