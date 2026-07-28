#include "digest_search_common.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::vector<std::uint8_t> left{0x00U, 0x00U, 0x00U};
    std::vector<std::uint8_t> right{0x00U, 0x00U, 0x01U};
    std::size_t samples = 10000U;
    std::size_t suffix_bytes = 2U;
    bool independent_suffix = true;
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
        } else if (arg == "--samples" && i + 1 < argc) {
            options.samples = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--suffix-bytes" && i + 1 < argc) {
            options.suffix_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--common-suffix") {
            options.independent_suffix = false;
        } else if (arg == "--independent-suffix") {
            options.independent_suffix = true;
        } else {
            throw std::invalid_argument(
                "usage: pvc-barrier-correlation [--preset NAME] "
                "[--left HEX] [--right HEX] [--samples N] "
                "[--suffix-bytes 1..4] [--common-suffix|--independent-suffix]");
        }
    }
    if (options.left.size() != options.right.size() || options.left.empty()) {
        throw std::invalid_argument("left and right prefixes must have equal non-zero length");
    }
    if (options.samples == 0U || options.samples > 1000000U) {
        throw std::invalid_argument("samples must be in [1,1000000]");
    }
    if (options.suffix_bytes == 0U || options.suffix_bytes > 4U) {
        throw std::invalid_argument("suffix-bytes must be in [1,4]");
    }
    return options;
}

std::vector<std::uint8_t> append_suffix(
    const std::vector<std::uint8_t>& prefix,
    std::uint64_t value,
    std::size_t bytes) {
    auto message = prefix;
    for (std::size_t i = 0U; i < bytes; ++i) {
        const auto shift = static_cast<unsigned>((bytes - 1U - i) * 8U);
        message.push_back(static_cast<std::uint8_t>(value >> shift));
    }
    return message;
}

struct Accumulator {
    long double sum_x = 0.0L;
    long double sum_y = 0.0L;
    long double sum_x2 = 0.0L;
    long double sum_y2 = 0.0L;
    long double sum_xy = 0.0L;
    std::size_t count = 0U;
    std::size_t min_x = std::numeric_limits<std::size_t>::max();
    std::size_t max_x = 0U;

    void add(std::size_t x, std::size_t y) {
        const auto lx = static_cast<long double>(x);
        const auto ly = static_cast<long double>(y);
        sum_x += lx;
        sum_y += ly;
        sum_x2 += lx * lx;
        sum_y2 += ly * ly;
        sum_xy += lx * ly;
        ++count;
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
    }

    [[nodiscard]] long double mean_x() const {
        return sum_x / static_cast<long double>(count);
    }

    [[nodiscard]] long double correlation() const {
        const auto n = static_cast<long double>(count);
        const auto numerator = n * sum_xy - sum_x * sum_y;
        const auto left = n * sum_x2 - sum_x * sum_x;
        const auto right = n * sum_y2 - sum_y * sum_y;
        const auto denominator = std::sqrt(std::max(0.0L, left * right));
        return denominator == 0.0L ? 0.0L : numerator / denominator;
    }
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        constexpr std::array<pvc::ResearchPhase, 5> phases{
            pvc::ResearchPhase::AfterForward,
            pvc::ResearchPhase::AfterFoldback,
            pvc::ResearchPhase::AfterDiagonalClosure,
            pvc::ResearchPhase::AfterOrbitClosure,
            pvc::ResearchPhase::Final,
        };
        std::array<Accumulator, phases.size()> accumulators{};
        std::size_t exact_digest_collisions = 0U;
        std::size_t forward_equal = 0U;
        std::size_t min_digest = std::numeric_limits<std::size_t>::max();
        std::size_t max_digest = 0U;
        long double digest_sum = 0.0L;

        std::cout << "Correlation of internal phase distance with final digest distance\n"
                  << "preset=" << preset.name
                  << " left_prefix=" << pvc::tool::hex_bytes(options.left)
                  << " right_prefix=" << pvc::tool::hex_bytes(options.right)
                  << " samples=" << options.samples
                  << " suffix_bytes=" << options.suffix_bytes
                  << " suffix_mode="
                  << (options.independent_suffix ? "independent" : "common")
                  << '\n';
        pvc::tool::print_parameters(preset.parameters);

        const std::uint64_t mask = options.suffix_bytes == 4U
            ? 0xffffffffULL
            : (1ULL << static_cast<unsigned>(options.suffix_bytes * 8U)) - 1ULL;
        for (std::size_t sample = 0U; sample < options.samples; ++sample) {
            const auto left_value = static_cast<std::uint64_t>(sample) & mask;
            const auto right_value = options.independent_suffix
                ? (static_cast<std::uint64_t>(sample) * 0x9e3779b1ULL
                   + 0x7f4a7c15ULL) & mask
                : left_value;
            const auto left_message = append_suffix(
                options.left, left_value, options.suffix_bytes);
            const auto right_message = append_suffix(
                options.right, right_value, options.suffix_bytes);
            const auto left_result = pvc::inspect_with_parameters(
                left_message, preset.parameters, false, true);
            const auto right_result = pvc::inspect_with_parameters(
                right_message, preset.parameters, false, true);
            const auto digest_distance = pvc::tool::byte_hamming_distance(
                left_result.digest, right_result.digest);
            digest_sum += static_cast<long double>(digest_distance);
            min_digest = std::min(min_digest, digest_distance);
            max_digest = std::max(max_digest, digest_distance);
            exact_digest_collisions += digest_distance == 0U ? 1U : 0U;

            for (std::size_t i = 0U; i < phases.size(); ++i) {
                const auto& left_state = pvc::tool::phase_state(
                    left_result, phases[i]);
                const auto& right_state = pvc::tool::phase_state(
                    right_result, phases[i]);
                const auto state_distance = pvc::tool::operational_state_bit_distance(
                    left_state, right_state);
                accumulators[i].add(state_distance, digest_distance);
                if (phases[i] == pvc::ResearchPhase::AfterForward
                    && state_distance == 0U) {
                    ++forward_equal;
                }
            }
        }

        std::cout << std::fixed << std::setprecision(6)
                  << "digest_mean_bits="
                  << digest_sum / static_cast<long double>(options.samples) << '\n'
                  << "digest_min_bits=" << min_digest << '\n'
                  << "digest_max_bits=" << max_digest << '\n'
                  << "exact_digest_collisions=" << exact_digest_collisions << '\n'
                  << "forward_equal_samples=" << forward_equal << '\n';
        for (std::size_t i = 0U; i < phases.size(); ++i) {
            std::cout << "phase=" << pvc::research_phase_name(phases[i])
                      << " mean_state_bits=" << accumulators[i].mean_x()
                      << " min_state_bits=" << accumulators[i].min_x
                      << " max_state_bits=" << accumulators[i].max_x
                      << " digest_correlation=" << accumulators[i].correlation()
                      << '\n';
        }
        return exact_digest_collisions == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
