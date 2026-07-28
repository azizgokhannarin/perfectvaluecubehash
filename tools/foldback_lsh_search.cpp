#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::array<std::uint8_t, 3> left{{0x17U, 0x6fU, 0x00U}};
    std::array<std::uint8_t, 3> right{{0x17U, 0x99U, 0x00U}};
    bool prefixes_set = false;
    std::size_t suffix_bytes = 2U;
    std::size_t projections = 16U;
    std::size_t projection_bytes = 4U;
    std::uint32_t suffix_limit = 0U;
    std::size_t print_limit = 8U;
};

std::uint8_t hex_nibble(char value) {
    if (value >= '0' && value <= '9') {
        return static_cast<std::uint8_t>(value - '0');
    }
    if (value >= 'a' && value <= 'f') {
        return static_cast<std::uint8_t>(10 + value - 'a');
    }
    if (value >= 'A' && value <= 'F') {
        return static_cast<std::uint8_t>(10 + value - 'A');
    }
    throw std::invalid_argument("invalid hex digit");
}

std::array<std::uint8_t, 3> parse_prefix(std::string_view text) {
    if (text.size() != 6U) {
        throw std::invalid_argument("prefix must contain exactly 3 bytes");
    }
    std::array<std::uint8_t, 3> result{};
    for (std::size_t i = 0U; i < result.size(); ++i) {
        result[i] = static_cast<std::uint8_t>(
            (hex_nibble(text[i * 2U]) << 4U)
            | hex_nibble(text[i * 2U + 1U]));
    }
    return result;
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--left" && i + 1 < argc) {
            options.left = parse_prefix(argv[++i]);
            options.prefixes_set = true;
        } else if (arg == "--right" && i + 1 < argc) {
            options.right = parse_prefix(argv[++i]);
            options.prefixes_set = true;
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--projections" && i + 1 < argc) {
            options.projections = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--projection-bytes" && i + 1 < argc) {
            options.projection_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-limit" && i + 1 < argc) {
            options.suffix_limit = static_cast<std::uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-foldback-lsh-search [--preset NAME] "
                "--left HEX6 --right HEX6 [--suffix-bytes 1|2] "
                "[--projections N] [--projection-bytes 1..8] "
                "[--suffix-limit N] [--print-limit N]");
        }
    }
    if (!options.prefixes_set) {
        options.left = {{0x17U, 0x6fU, 0x00U}};
        options.right = {{0x17U, 0x99U, 0x00U}};
    }
    if (options.suffix_bytes == 0U || options.suffix_bytes > 2U) {
        throw std::invalid_argument("suffix-bytes must be 1 or 2");
    }
    if (options.projections == 0U || options.projections > 256U) {
        throw std::invalid_argument("projections must be in [1,256]");
    }
    if (options.projection_bytes == 0U || options.projection_bytes > 8U) {
        throw std::invalid_argument("projection-bytes must be in [1,8]");
    }
    const std::uint32_t maximum = options.suffix_bytes == 1U ? 256U : 65536U;
    if (options.suffix_limit == 0U) {
        options.suffix_limit = maximum;
    }
    if (options.suffix_limit > maximum) {
        throw std::invalid_argument("suffix-limit exceeds selected suffix domain");
    }
    return options;
}

std::vector<std::uint8_t> make_message(
    const std::array<std::uint8_t, 3>& prefix,
    std::uint32_t suffix,
    std::size_t suffix_bytes) {
    std::vector<std::uint8_t> message(prefix.begin(), prefix.end());
    for (std::size_t i = 0U; i < suffix_bytes; ++i) {
        const auto shift = static_cast<unsigned>((suffix_bytes - 1U - i) * 8U);
        message.push_back(static_cast<std::uint8_t>(suffix >> shift));
    }
    return message;
}

struct Record {
    std::uint32_t suffix = 0U;
    pvc::InternalStateSnapshot state{};
};

pvc::InternalStateSnapshot evaluate_suffix(
    const pvc::InternalStateSnapshot& common_prefix_state,
    const std::array<std::uint8_t, 3>& prefix,
    std::uint32_t suffix,
    std::size_t suffix_bytes,
    const pvc::HashParameters& parameters) {
    auto forward = common_prefix_state;
    for (std::size_t i = 0U; i < suffix_bytes; ++i) {
        const auto shift = static_cast<unsigned>((suffix_bytes - 1U - i) * 8U);
        forward = pvc::absorb_symbol_for_research(
            forward, static_cast<std::uint8_t>(suffix >> shift), parameters);
    }
    const auto message = make_message(prefix, suffix, suffix_bytes);
    return pvc::foldback_from_forward_state_for_research(
        forward, message, parameters);
}

std::uint64_t projection_key(const pvc::InternalStateSnapshot& state,
                             std::size_t pass,
                             std::size_t bytes) {
    const auto& storage = state.cube.storage();
    std::uint64_t key = 0xcbf29ce484222325ULL;
    for (std::size_t i = 0U; i < bytes; ++i) {
        const std::size_t position =
            (pass * 73U + i * 97U + i * i * 29U + 19U) & 511U;
        key ^= static_cast<std::uint64_t>(storage[position])
             + static_cast<std::uint64_t>(position << 8U);
        key *= 0x100000001b3ULL;
    }
    return key;
}


struct BestPair {
    std::size_t bits = std::numeric_limits<std::size_t>::max();
    std::size_t bytes = std::numeric_limits<std::size_t>::max();
    std::uint32_t left_suffix = 0U;
    std::uint32_t right_suffix = 0U;
    bool exact = false;
};

void consider(BestPair& best,
              const Record& left,
              const Record& right) {
    const auto bits = pvc::tool::operational_state_bit_distance(
        left.state, right.state);
    const auto bytes = pvc::tool::operational_state_byte_distance(
        left.state, right.state);
    if (bits < best.bits || (bits == best.bits && bytes < best.bytes)) {
        best = BestPair{
            .bits = bits,
            .bytes = bytes,
            .left_suffix = left.suffix,
            .right_suffix = right.suffix,
            .exact = left.state == right.state,
        };
    }
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const std::uint32_t suffix_count = options.suffix_limit;
        const auto left_prefix_state = pvc::forward_state_for_research(
            options.left, preset.parameters);
        const auto right_prefix_state = pvc::forward_state_for_research(
            options.right, preset.parameters);
        if (left_prefix_state != right_prefix_state) {
            throw std::invalid_argument(
                "the selected prefixes do not share an exact forward state");
        }

        std::cout << "Projection-LSH search for independent suffixes that minimize "
                     "after-foldback distance\n"
                  << "preset=" << preset.name
                  << " left=" << pvc::tool::hex_bytes(
                        std::vector<std::uint8_t>(options.left.begin(), options.left.end()))
                  << " right=" << pvc::tool::hex_bytes(
                        std::vector<std::uint8_t>(options.right.begin(), options.right.end()))
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffixes_per_side=" << suffix_count
                  << " logical_cross_pairs="
                  << static_cast<std::uint64_t>(suffix_count) * suffix_count
                  << " projections=" << options.projections
                  << " projection_bytes=" << options.projection_bytes << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<Record> left_records;
        std::vector<Record> right_records;
        left_records.reserve(suffix_count);
        right_records.reserve(suffix_count);
        for (std::uint32_t suffix = 0U; suffix < suffix_count; ++suffix) {
            left_records.push_back(Record{
                .suffix = suffix,
                .state = evaluate_suffix(
                    left_prefix_state, options.left, suffix,
                    options.suffix_bytes, preset.parameters),
            });
            right_records.push_back(Record{
                .suffix = suffix,
                .state = evaluate_suffix(
                    right_prefix_state, options.right, suffix,
                    options.suffix_bytes, preset.parameters),
            });
        }

        BestPair best;
        std::uint64_t candidate_pairs = 0U;
        for (std::size_t pass = 0U; pass < options.projections; ++pass) {
            std::unordered_map<std::uint64_t, std::vector<std::uint32_t>> buckets;
            buckets.reserve(suffix_count * 2U);
            for (std::uint32_t index = 0U; index < suffix_count; ++index) {
                buckets[projection_key(
                    left_records[index].state, pass, options.projection_bytes)]
                    .push_back(index);
            }
            for (const auto& right : right_records) {
                const auto found = buckets.find(projection_key(
                    right.state, pass, options.projection_bytes));
                if (found == buckets.end()) {
                    continue;
                }
                candidate_pairs += found->second.size();
                for (const auto index : found->second) {
                    consider(best, left_records[index], right);
                }
            }
            std::cout << "projection=" << pass
                      << " cumulative_candidates=" << candidate_pairs
                      << " best_bits=" << best.bits
                      << " best_bytes=" << best.bytes << '\n';
            if (best.exact) {
                break;
            }
        }

        const auto left_message = make_message(
            options.left, best.left_suffix, options.suffix_bytes);
        const auto right_message = make_message(
            options.right, best.right_suffix, options.suffix_bytes);
        const auto left_digest = pvc::hash_with_parameters(
            left_message, preset.parameters);
        const auto right_digest = pvc::hash_with_parameters(
            right_message, preset.parameters);
        std::cout << "candidate_pairs_evaluated=" << candidate_pairs << '\n'
                  << "best_foldback_bits=" << best.bits << '\n'
                  << "best_foldback_bytes=" << best.bytes << '\n'
                  << "best_digest_bits=" << pvc::tool::byte_hamming_distance(
                        left_digest, right_digest) << '\n'
                  << "left_message=" << pvc::tool::hex_bytes(left_message) << '\n'
                  << "right_message=" << pvc::tool::hex_bytes(right_message) << '\n'
                  << "exact_after_foldback_collision="
                  << (best.exact ? "yes" : "no") << '\n';
        return best.exact ? 1 : 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
