// Phase 1-a: sample after-foldback distances across many independent forward
// collision seeds (not a single bridged path). Frozen candidate only.

#include "alias_catalog.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t prefix_count = 65536U;
    std::size_t threads = 0U;
    std::size_t seed_limit = 64U;
    std::size_t suffix_bytes = 1U;
    std::uint64_t suffix_samples = 64U;
    std::size_t print_limit = 8U;
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
        } else if (arg == "--seed-limit" && i + 1 < argc) {
            options.seed_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-samples" && i + 1 < argc) {
            options.suffix_samples = std::stoull(argv[++i]);
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-multipath-foldback-sample [--preset NAME] "
                "[--prefix-count 1..65536] [--threads N] [--seed-limit N] "
                "[--suffix-bytes 0..2] [--suffix-samples N] [--print-limit N]");
        }
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    if (options.seed_limit == 0U || options.seed_limit > 100000U) {
        throw std::invalid_argument("seed-limit must be in [1,100000]");
    }
    if (options.suffix_bytes > 2U) {
        throw std::invalid_argument("suffix-bytes must be in [0,2]");
    }
    return options;
}

std::vector<std::uint8_t> sample_suffix(std::uint64_t counter, std::size_t bytes) {
    std::vector<std::uint8_t> suffix(bytes);
    std::uint64_t state = counter * 0x9e3779b97f4a7c15ULL + 0xbf58476d1ce4e5b9ULL;
    for (std::size_t i = 0; i < bytes; ++i) {
        state ^= state >> 12U;
        state ^= state << 25U;
        state ^= state >> 27U;
        state *= 0x2545f4914f6cdd1dULL;
        suffix[i] = static_cast<std::uint8_t>(state >> ((i & 7U) * 8U));
    }
    return suffix;
}

std::vector<std::uint8_t> extend(const std::array<std::uint8_t, 3>& prefix,
                                 const std::vector<std::uint8_t>& suffix) {
    std::vector<std::uint8_t> out(prefix.begin(), prefix.end());
    out.insert(out.end(), suffix.begin(), suffix.end());
    return out;
}

double mean(const std::vector<std::size_t>& values) {
    if (values.empty()) {
        return 0.0;
    }
    const double sum = std::accumulate(
        values.begin(), values.end(), 0.0,
        [](double a, std::size_t b) { return a + static_cast<double>(b); });
    return sum / static_cast<double>(values.size());
}

std::size_t percentile(std::vector<std::size_t> values, double p) {
    if (values.empty()) {
        return 0U;
    }
    std::sort(values.begin(), values.end());
    const auto idx = static_cast<std::size_t>(
        std::clamp(p, 0.0, 1.0) * static_cast<double>(values.size() - 1U));
    return values[idx];
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);

        std::cout << "Multi-path foldback distance sample (Phase 1-a)\n"
                  << "preset=" << preset.name
                  << " seed_limit=" << options.seed_limit
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffix_samples=" << options.suffix_samples << '\n';
        pvc::tool::print_parameters(preset.parameters);

        auto pairs = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);
        if (pairs.size() > options.seed_limit) {
            pairs.resize(options.seed_limit);
        }
        std::cout << "seeds=" << pairs.size() << '\n';

        std::vector<std::size_t> bare_bits;
        std::vector<std::size_t> best_suffix_bits;
        bare_bits.reserve(pairs.size());
        best_suffix_bits.reserve(pairs.size());

        std::size_t global_min = std::numeric_limits<std::size_t>::max();
        std::size_t exact_merges = 0U;
        std::size_t printed = 0U;
        std::string best_left;
        std::string best_right;

        for (std::size_t seed = 0; seed < pairs.size(); ++seed) {
            const auto& pair = pairs[seed];
            const auto left_bare = pvc::tool::as_vector(pair.left);
            const auto right_bare = pvc::tool::as_vector(pair.right);
            const auto left_fb = pvc::foldback_state_for_research(
                left_bare, preset.parameters);
            const auto right_fb = pvc::foldback_state_for_research(
                right_bare, preset.parameters);
            const auto bare = pvc::tool::operational_state_bit_distance(
                left_fb, right_fb);
            bare_bits.push_back(bare);
            if (left_fb == right_fb) {
                ++exact_merges;
            }

            std::size_t seed_min = bare;
            std::vector<std::uint8_t> min_left = left_bare;
            std::vector<std::uint8_t> min_right = right_bare;

            const std::uint64_t samples = options.suffix_bytes == 0U
                ? 1U
                : options.suffix_samples;
            for (std::uint64_t s = 0; s < samples; ++s) {
                const auto suffix = options.suffix_bytes == 0U
                    ? std::vector<std::uint8_t>{}
                    : sample_suffix(seed * 0x100000001ULL + s, options.suffix_bytes);
                const auto left_msg = extend(pair.left, suffix);
                const auto right_msg = extend(pair.right, suffix);
                // Common suffix on a forward collision keeps one forward state.
                const auto lf = pvc::foldback_state_for_research(
                    left_msg, preset.parameters);
                const auto rf = pvc::foldback_state_for_research(
                    right_msg, preset.parameters);
                const auto bits = pvc::tool::operational_state_bit_distance(lf, rf);
                if (bits < seed_min) {
                    seed_min = bits;
                    min_left = left_msg;
                    min_right = right_msg;
                }
                if (lf == rf) {
                    ++exact_merges;
                    if (printed < options.print_limit) {
                        std::cout << "exact_merge left=" << pvc::tool::hex_bytes(left_msg)
                                  << " right=" << pvc::tool::hex_bytes(right_msg) << '\n';
                        ++printed;
                    }
                }
            }

            best_suffix_bits.push_back(seed_min);
            if (seed_min < global_min) {
                global_min = seed_min;
                best_left = pvc::tool::hex_bytes(min_left);
                best_right = pvc::tool::hex_bytes(min_right);
            }
        }

        std::cout << "bare_after_foldback_bits_mean=" << mean(bare_bits) << '\n'
                  << "bare_after_foldback_bits_p50=" << percentile(bare_bits, 0.50) << '\n'
                  << "bare_after_foldback_bits_p10=" << percentile(bare_bits, 0.10) << '\n'
                  << "bare_after_foldback_bits_min="
                  << (bare_bits.empty()
                          ? 0U
                          : *std::min_element(bare_bits.begin(), bare_bits.end()))
                  << '\n'
                  << "best_common_suffix_bits_mean=" << mean(best_suffix_bits) << '\n'
                  << "best_common_suffix_bits_p50="
                  << percentile(best_suffix_bits, 0.50) << '\n'
                  << "best_common_suffix_bits_p10="
                  << percentile(best_suffix_bits, 0.10) << '\n'
                  << "global_minimum_after_foldback_bits=" << global_min << '\n'
                  << "global_minimum_left=" << best_left << '\n'
                  << "global_minimum_right=" << best_right << '\n'
                  << "exact_after_foldback_merges=" << exact_merges << '\n'
                  << "note=distances are full operational-state Hamming bits; "
                     "exact merge would be a Phase-1 break\n";

        return exact_merges == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
