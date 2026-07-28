#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
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
    std::size_t samples = 4;
    std::size_t message_bytes = 16;
    std::string mode = "single";
};

struct PhaseMetrics {
    pvc::ResearchPhase phase = pvc::ResearchPhase::Initial;
    std::size_t index = 0;
    std::uint64_t cube_bit_sum = 0;
    std::uint64_t cube_byte_sum = 0;
    std::size_t cube_bit_min = std::numeric_limits<std::size_t>::max();
    std::size_t cube_bit_max = 0;
    std::size_t cube_byte_min = std::numeric_limits<std::size_t>::max();
    std::size_t cube_byte_max = 0;
    std::size_t exact_state_matches = 0;
    std::size_t count = 0;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--samples" && i + 1 < argc) {
            options.samples = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--message-bytes" && i + 1 < argc) {
            options.message_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--mode" && i + 1 < argc) {
            options.mode = argv[++i];
        } else {
            throw std::invalid_argument(
                "usage: pvc-differential-search [--preset NAME] [--samples N] "
                "[--message-bytes N>=16] [--mode single|paired]");
        }
    }
    if (options.message_bytes < 16U) {
        throw std::invalid_argument("message-bytes must be at least 16");
    }
    if (options.mode != "single" && options.mode != "paired") {
        throw std::invalid_argument("mode must be single or paired");
    }
    return options;
}

void update_metric(PhaseMetrics& metric,
                   const pvc::InternalStateSnapshot& left,
                   const pvc::InternalStateSnapshot& right) {
    const auto bit_distance = pvc::tool::cube_bit_distance(left.cube, right.cube);
    const auto byte_distance = pvc::tool::cube_byte_distance(left.cube, right.cube);
    metric.cube_bit_sum += bit_distance;
    metric.cube_byte_sum += byte_distance;
    metric.cube_bit_min = std::min(metric.cube_bit_min, bit_distance);
    metric.cube_bit_max = std::max(metric.cube_bit_max, bit_distance);
    metric.cube_byte_min = std::min(metric.cube_byte_min, byte_distance);
    metric.cube_byte_max = std::max(metric.cube_byte_max, byte_distance);
    if (left == right) {
        ++metric.exact_state_matches;
    }
    ++metric.count;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);

        std::cout << "Differential phase search\n"
                  << "preset=" << preset.name
                  << " samples=" << options.samples
                  << " message_bytes=" << options.message_bytes
                  << " mode=" << options.mode << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<PhaseMetrics> metrics;
        std::uint64_t digest_bit_sum = 0;
        std::size_t digest_bit_min = std::numeric_limits<std::size_t>::max();
        std::size_t digest_bit_max = 0;
        std::size_t digest_exact_matches = 0;
        std::size_t comparisons = 0;

        for (std::size_t sample = 0; sample < options.samples; ++sample) {
            const auto base = pvc::tool::counter_message(
                static_cast<std::uint64_t>(sample), 0x444946464552454EULL,
                options.message_bytes);
            const auto base_result = pvc::inspect_with_parameters(
                base, preset.parameters, false, true);

            const std::size_t mutation_count = options.mode == "single"
                                             ? options.message_bytes * 8U
                                             : options.message_bytes * 8U;
            for (std::size_t mutation = 0; mutation < mutation_count; ++mutation) {
                auto changed = base;
                const std::size_t byte_index = mutation / 8U;
                const auto bit = static_cast<unsigned>(mutation & 7U);
                changed[byte_index] ^= static_cast<std::uint8_t>(1U << bit);
                if (options.mode == "paired") {
                    const std::size_t second = (byte_index + options.message_bytes / 2U)
                                             % options.message_bytes;
                    changed[second] ^= static_cast<std::uint8_t>(1U << bit);
                }

                const auto changed_result = pvc::inspect_with_parameters(
                    changed, preset.parameters, false, true);
                if (changed_result.checkpoints.size() != base_result.checkpoints.size()) {
                    throw std::logic_error("checkpoint count changed for equal-length inputs");
                }
                if (metrics.empty()) {
                    metrics.reserve(base_result.checkpoints.size());
                    for (const auto& checkpoint : base_result.checkpoints) {
                        metrics.push_back(PhaseMetrics{
                            .phase = checkpoint.phase,
                            .index = checkpoint.index,
                        });
                    }
                }

                for (std::size_t i = 0; i < metrics.size(); ++i) {
                    const auto& left = base_result.checkpoints[i];
                    const auto& right = changed_result.checkpoints[i];
                    if (left.phase != right.phase || left.index != right.index) {
                        throw std::logic_error("checkpoint sequence mismatch");
                    }
                    update_metric(metrics[i], left.state, right.state);
                }

                const auto digest_distance = pvc::tool::byte_hamming_distance(
                    base_result.digest, changed_result.digest);
                digest_bit_sum += digest_distance;
                digest_bit_min = std::min(digest_bit_min, digest_distance);
                digest_bit_max = std::max(digest_bit_max, digest_distance);
                if (base_result.digest == changed_result.digest) {
                    ++digest_exact_matches;
                }
                ++comparisons;
            }
        }

        std::cout << "\nphase,index,mean_cube_bits,min,max,mean_cube_bytes,min,max,exact_states\n";
        for (const auto& metric : metrics) {
            const auto mean_bits = static_cast<double>(metric.cube_bit_sum)
                                 / static_cast<double>(metric.count);
            const auto mean_bytes = static_cast<double>(metric.cube_byte_sum)
                                  / static_cast<double>(metric.count);
            std::cout << pvc::research_phase_name(metric.phase) << ','
                      << metric.index << ','
                      << std::fixed << std::setprecision(4) << mean_bits << ','
                      << metric.cube_bit_min << ',' << metric.cube_bit_max << ','
                      << mean_bytes << ','
                      << metric.cube_byte_min << ',' << metric.cube_byte_max << ','
                      << metric.exact_state_matches << '\n';
        }

        std::cout << "\ndigest comparisons=" << comparisons
                  << " mean_bits=" << std::fixed << std::setprecision(4)
                  << (static_cast<double>(digest_bit_sum)
                      / static_cast<double>(comparisons))
                  << " min=" << digest_bit_min
                  << " max=" << digest_bit_max
                  << " exact_digest_matches=" << digest_exact_matches << '\n';
        return digest_exact_matches == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
