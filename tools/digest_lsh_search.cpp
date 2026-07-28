#include "digest_search_common.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::vector<std::uint8_t> left{0x00U, 0x00U, 0x00U};
    std::vector<std::uint8_t> right{0x00U, 0x00U, 0x01U};
    std::size_t suffix_bytes = 2U;
    std::uint32_t suffix_limit = 8192U;
    std::size_t projections = 32U;
    std::size_t projection_bytes = 1U;
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
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-limit" && i + 1 < argc) {
            options.suffix_limit = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--projections" && i + 1 < argc) {
            options.projections = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--projection-bytes" && i + 1 < argc) {
            options.projection_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-digest-lsh-search [--preset NAME] "
                "[--left HEX] [--right HEX] [--suffix-bytes 1|2] "
                "[--suffix-limit N] [--projections N] [--projection-bytes 1..4]");
        }
    }
    if (options.left.size() != options.right.size() || options.left.empty()) {
        throw std::invalid_argument("left and right prefixes must have equal non-zero length");
    }
    if (options.suffix_bytes == 0U || options.suffix_bytes > 2U) {
        throw std::invalid_argument("suffix-bytes must be 1 or 2");
    }
    const std::uint32_t maximum = options.suffix_bytes == 1U ? 256U : 65536U;
    if (options.suffix_limit == 0U || options.suffix_limit > maximum) {
        throw std::invalid_argument("suffix-limit exceeds selected suffix domain");
    }
    if (options.projections == 0U || options.projections > 256U) {
        throw std::invalid_argument("projections must be in [1,256]");
    }
    if (options.projection_bytes == 0U || options.projection_bytes > 4U) {
        throw std::invalid_argument("projection-bytes must be in [1,4]");
    }
    return options;
}

std::vector<std::uint8_t> make_message(
    const std::vector<std::uint8_t>& prefix,
    std::uint32_t suffix,
    std::size_t suffix_bytes) {
    auto message = prefix;
    for (std::size_t i = 0U; i < suffix_bytes; ++i) {
        const auto shift = static_cast<unsigned>((suffix_bytes - 1U - i) * 8U);
        message.push_back(static_cast<std::uint8_t>(suffix >> shift));
    }
    return message;
}

struct Record {
    std::uint32_t suffix = 0U;
    std::vector<std::uint8_t> digest;
    pvc::InternalStateSnapshot forward{};
};

std::uint64_t projection_key(std::span<const std::uint8_t> digest,
                             std::size_t pass,
                             std::size_t bytes) {
    std::uint64_t key = 0U;
    for (std::size_t i = 0U; i < bytes; ++i) {
        const auto position = (pass * 11U + i * 17U + i * i * 3U + 5U) & 31U;
        key = (key << 8U) | digest[position];
    }
    return key;
}

struct BestPair {
    std::size_t bits = std::numeric_limits<std::size_t>::max();
    std::uint32_t left_suffix = 0U;
    std::uint32_t right_suffix = 0U;
    bool exact = false;
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto left_prefix_state = pvc::forward_state_for_research(
            options.left, preset.parameters);
        const auto right_prefix_state = pvc::forward_state_for_research(
            options.right, preset.parameters);
        if (left_prefix_state == right_prefix_state) {
            throw std::invalid_argument(
                "prefixes already share a forward state; this tool requires divergent prefixes");
        }

        std::cout << "Digest-LSH search across different forward-state families\n"
                  << "preset=" << preset.name
                  << " left_prefix=" << pvc::tool::hex_bytes(options.left)
                  << " right_prefix=" << pvc::tool::hex_bytes(options.right)
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffixes_per_side=" << options.suffix_limit
                  << " logical_cross_pairs="
                  << static_cast<std::uint64_t>(options.suffix_limit)
                        * options.suffix_limit
                  << " projections=" << options.projections
                  << " projection_bytes=" << options.projection_bytes << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<Record> left_records;
        std::vector<Record> right_records;
        left_records.reserve(options.suffix_limit);
        right_records.reserve(options.suffix_limit);
        for (std::uint32_t suffix = 0U; suffix < options.suffix_limit; ++suffix) {
            const auto left_message = make_message(
                options.left, suffix, options.suffix_bytes);
            const auto right_message = make_message(
                options.right, suffix, options.suffix_bytes);
            left_records.push_back(Record{
                .suffix = suffix,
                .digest = pvc::hash_with_parameters(left_message, preset.parameters),
                .forward = pvc::forward_state_for_research(
                    left_message, preset.parameters),
            });
            right_records.push_back(Record{
                .suffix = suffix,
                .digest = pvc::hash_with_parameters(right_message, preset.parameters),
                .forward = pvc::forward_state_for_research(
                    right_message, preset.parameters),
            });
        }

        BestPair best;
        std::uint64_t candidate_pairs = 0U;
        std::uint64_t forward_equal_skipped = 0U;
        for (std::size_t pass = 0U; pass < options.projections; ++pass) {
            std::unordered_map<std::uint64_t, std::vector<std::uint32_t>> buckets;
            buckets.reserve(options.suffix_limit * 2U);
            for (std::uint32_t i = 0U; i < options.suffix_limit; ++i) {
                buckets[projection_key(
                    left_records[i].digest, pass, options.projection_bytes)]
                    .push_back(i);
            }
            for (const auto& right : right_records) {
                const auto found = buckets.find(projection_key(
                    right.digest, pass, options.projection_bytes));
                if (found == buckets.end()) {
                    continue;
                }
                for (const auto index : found->second) {
                    ++candidate_pairs;
                    const auto& left = left_records[index];
                    if (left.forward == right.forward) {
                        ++forward_equal_skipped;
                        continue;
                    }
                    const auto bits = pvc::tool::byte_hamming_distance(
                        left.digest, right.digest);
                    if (bits < best.bits) {
                        best = BestPair{
                            .bits = bits,
                            .left_suffix = left.suffix,
                            .right_suffix = right.suffix,
                            .exact = bits == 0U,
                        };
                    }
                }
            }
            std::cout << "projection=" << pass
                      << " cumulative_candidates=" << candidate_pairs
                      << " best_digest_bits=" << best.bits << '\n';
            if (best.exact) {
                break;
            }
        }

        if (best.bits == std::numeric_limits<std::size_t>::max()) {
            throw std::runtime_error("no LSH candidate pair was produced");
        }
        const auto left_message = make_message(
            options.left, best.left_suffix, options.suffix_bytes);
        const auto right_message = make_message(
            options.right, best.right_suffix, options.suffix_bytes);
        const auto logical_pairs = static_cast<std::uint64_t>(options.suffix_limit)
                                 * options.suffix_limit;
        std::cout << "candidate_pairs_evaluated=" << candidate_pairs << '\n'
                  << "forward_equal_skipped=" << forward_equal_skipped << '\n'
                  << "generic_cross_min_digest_bits="
                  << pvc::tool::generic_minimum_distance(logical_pairs) << '\n'
                  << "best_digest_bits=" << best.bits << '\n'
                  << "left_message=" << pvc::tool::hex_bytes(left_message) << '\n'
                  << "right_message=" << pvc::tool::hex_bytes(right_message) << '\n'
                  << "exact_digest_collision=" << (best.exact ? "yes" : "no") << '\n';
        pvc::tool::print_phase_distance_profile(
            left_message, right_message, preset.parameters);
        return best.exact ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
