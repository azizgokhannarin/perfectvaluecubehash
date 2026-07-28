#include "digest_search_common.hpp"
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
    std::vector<std::uint8_t> left{0x00U, 0x00U, 0x00U};
    std::vector<std::uint8_t> right{0x00U, 0x00U, 0x01U};
    std::size_t depth = 8U;
    std::size_t beam_width = 128U;
    std::size_t branch = 16U;
    std::size_t print_limit = 8U;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--left" && i + 1 < argc) {
            options.left = pvc::tool::parse_hex_message(argv[++i]);
        } else if (arg == "--right" && i + 1 < argc) {
            options.right = pvc::tool::parse_hex_message(argv[++i]);
        } else if (arg == "--depth" && i + 1 < argc) {
            options.depth = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--beam" && i + 1 < argc) {
            options.beam_width = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--branch" && i + 1 < argc) {
            options.branch = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-divergent-digest-beam [--preset NAME] "
                "[--left HEX] [--right HEX] [--depth 1..32] "
                "[--beam N] [--branch 2..32] [--print-limit N]");
        }
    }
    if (options.left.size() != options.right.size() || options.left.empty()) {
        throw std::invalid_argument("left and right seeds must have equal non-zero length");
    }
    if (options.depth == 0U || options.depth > 32U) {
        throw std::invalid_argument("depth must be in [1,32]");
    }
    if (options.beam_width == 0U || options.beam_width > 4096U) {
        throw std::invalid_argument("beam must be in [1,4096]");
    }
    if (options.branch < 2U || options.branch > 32U) {
        throw std::invalid_argument("branch must be in [2,32]");
    }
    return options;
}

struct Candidate {
    std::vector<std::uint8_t> left;
    std::vector<std::uint8_t> right;
    std::vector<std::uint8_t> left_digest;
    std::vector<std::uint8_t> right_digest;
    pvc::InternalStateSnapshot left_forward{};
    pvc::InternalStateSnapshot right_forward{};
    std::size_t digest_bits = std::numeric_limits<std::size_t>::max();
    std::size_t forward_bits = std::numeric_limits<std::size_t>::max();
};

Candidate evaluate(std::vector<std::uint8_t> left,
                   std::vector<std::uint8_t> right,
                   const pvc::HashParameters& parameters) {
    auto left_digest = pvc::hash_with_parameters(left, parameters);
    auto right_digest = pvc::hash_with_parameters(right, parameters);
    auto left_forward = pvc::forward_state_for_research(left, parameters);
    auto right_forward = pvc::forward_state_for_research(right, parameters);
    const auto digest_bits = pvc::tool::byte_hamming_distance(
        left_digest, right_digest);
    const auto forward_bits = pvc::tool::operational_state_bit_distance(
        left_forward, right_forward);
    return Candidate{
        .left = std::move(left),
        .right = std::move(right),
        .left_digest = std::move(left_digest),
        .right_digest = std::move(right_digest),
        .left_forward = std::move(left_forward),
        .right_forward = std::move(right_forward),
        .digest_bits = digest_bits,
        .forward_bits = forward_bits,
    };
}

bool less_candidate(const Candidate& left, const Candidate& right) {
    if (left.digest_bits != right.digest_bits) {
        return left.digest_bits < right.digest_bits;
    }
    if (left.forward_bits != right.forward_bits) {
        return left.forward_bits > right.forward_bits;
    }
    if (left.left != right.left) {
        return left.left < right.left;
    }
    return left.right < right.right;
}

std::vector<std::uint8_t> palette(std::size_t branch,
                                  std::size_t depth,
                                  std::uint8_t salt) {
    std::vector<std::uint8_t> values;
    values.reserve(branch);
    for (std::size_t i = 0U; i < branch; ++i) {
        const auto spaced = static_cast<unsigned>((i * 256U) / branch);
        values.push_back(static_cast<std::uint8_t>(
            spaced + depth * 37U + static_cast<unsigned>(salt)));
    }
    return values;
}

std::string pair_key(const std::vector<std::uint8_t>& left,
                     const std::vector<std::uint8_t>& right) {
    std::string key;
    key.reserve(left.size() + right.size() + 1U);
    key.append(reinterpret_cast<const char*>(left.data()), left.size());
    key.push_back('\0');
    key.append(reinterpret_cast<const char*>(right.data()), right.size());
    return key;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        Candidate seed = evaluate(options.left, options.right, preset.parameters);
        if (seed.left_forward == seed.right_forward) {
            throw std::invalid_argument(
                "seed messages share a forward state; choose a forward-divergent pair");
        }

        std::cout << "Digest-guided beam over deliberately different forward states\n"
                  << "preset=" << preset.name
                  << " seed_left=" << pvc::tool::hex_bytes(options.left)
                  << " seed_right=" << pvc::tool::hex_bytes(options.right)
                  << " depth=" << options.depth
                  << " beam_width=" << options.beam_width
                  << " branch=" << options.branch << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<Candidate> beam;
        beam.push_back(seed);
        Candidate global_best = seed;
        std::size_t global_depth = 0U;
        std::uint64_t cumulative_evaluated = 1U;
        std::cout << "depth=0 best_digest_bits=" << seed.digest_bits
                  << " forward_bits=" << seed.forward_bits << '\n';

        for (std::size_t depth = 1U; depth <= options.depth; ++depth) {
            std::vector<Candidate> expanded;
            expanded.reserve(beam.size() * options.branch * options.branch);
            std::unordered_set<std::string> seen;
            seen.reserve(expanded.capacity() * 2U);
            const auto left_palette = palette(options.branch, depth, 0x13U);
            const auto right_palette = palette(options.branch, depth, 0xa7U);

            struct Child {
                std::vector<std::uint8_t> message;
                std::vector<std::uint8_t> digest;
                pvc::InternalStateSnapshot forward{};
            };

            for (const auto& candidate : beam) {
                std::vector<Child> left_children;
                std::vector<Child> right_children;
                left_children.reserve(options.branch);
                right_children.reserve(options.branch);

                for (const auto symbol : left_palette) {
                    auto message = candidate.left;
                    message.push_back(symbol);
                    auto digest = pvc::hash_with_parameters(message, preset.parameters);
                    auto forward = pvc::forward_state_for_research(
                        message, preset.parameters);
                    left_children.push_back(Child{
                        .message = std::move(message),
                        .digest = std::move(digest),
                        .forward = std::move(forward),
                    });
                }
                for (const auto symbol : right_palette) {
                    auto message = candidate.right;
                    message.push_back(symbol);
                    auto digest = pvc::hash_with_parameters(message, preset.parameters);
                    auto forward = pvc::forward_state_for_research(
                        message, preset.parameters);
                    right_children.push_back(Child{
                        .message = std::move(message),
                        .digest = std::move(digest),
                        .forward = std::move(forward),
                    });
                }

                for (const auto& left_child : left_children) {
                    for (const auto& right_child : right_children) {
                        if (left_child.forward == right_child.forward) {
                            continue;
                        }
                        const auto key = pair_key(
                            left_child.message, right_child.message);
                        if (!seen.insert(key).second) {
                            continue;
                        }
                        expanded.push_back(Candidate{
                            .left = left_child.message,
                            .right = right_child.message,
                            .left_digest = left_child.digest,
                            .right_digest = right_child.digest,
                            .left_forward = left_child.forward,
                            .right_forward = right_child.forward,
                            .digest_bits = pvc::tool::byte_hamming_distance(
                                left_child.digest, right_child.digest),
                            .forward_bits = pvc::tool::operational_state_bit_distance(
                                left_child.forward, right_child.forward),
                        });
                    }
                }
            }
            const std::uint64_t comparisons = expanded.size();
            cumulative_evaluated += comparisons;
            std::sort(expanded.begin(), expanded.end(), less_candidate);
            if (expanded.size() > options.beam_width) {
                expanded.resize(options.beam_width);
            }
            if (expanded.empty()) {
                throw std::runtime_error("divergent beam became empty");
            }
            beam = std::move(expanded);
            if (less_candidate(beam.front(), global_best)) {
                global_best = beam.front();
                global_depth = depth;
            }
            std::cout << "depth=" << depth
                      << " expanded=" << comparisons
                      << " generic_level_min_digest_bits="
                      << pvc::tool::generic_minimum_distance(comparisons)
                      << " generic_cumulative_min_digest_bits="
                      << pvc::tool::generic_minimum_distance(cumulative_evaluated)
                      << " best_digest_bits=" << beam.front().digest_bits
                      << " best_forward_bits=" << beam.front().forward_bits
                      << '\n';
            if (beam.front().digest_bits == 0U) {
                break;
            }
        }

        const auto limit = std::min(options.print_limit, beam.size());
        for (std::size_t i = 0U; i < limit; ++i) {
            std::cout << "rank=" << (i + 1U)
                      << " digest_bits=" << beam[i].digest_bits
                      << " forward_bits=" << beam[i].forward_bits
                      << " left=" << pvc::tool::hex_bytes(beam[i].left)
                      << " right=" << pvc::tool::hex_bytes(beam[i].right)
                      << '\n';
        }
        std::cout << "cumulative_pairs_evaluated=" << cumulative_evaluated << '\n'
                  << "generic_cumulative_min_digest_bits="
                  << pvc::tool::generic_minimum_distance(cumulative_evaluated) << '\n'
                  << "global_best_depth=" << global_depth << '\n'
                  << "global_best_digest_bits=" << global_best.digest_bits << '\n'
                  << "global_best_forward_bits=" << global_best.forward_bits << '\n'
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
