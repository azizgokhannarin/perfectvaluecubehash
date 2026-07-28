#include "alias_catalog.hpp"
#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
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
        } else if (arg == "--prefix-count" && i + 1 < argc) {
            options.prefix_count = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            options.threads = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-independent-suffix-catalog [--preset NAME] "
                "[--prefix-count 1..65536] [--threads N] [--print-limit N]");
        }
    }
    if (options.prefix_count == 0U || options.prefix_count > 65536U) {
        throw std::invalid_argument("prefix-count must be in [1,65536]");
    }
    if (options.threads == 0U) {
        options.threads = std::max<std::size_t>(1U, std::thread::hardware_concurrency());
    }
    return options;
}

std::vector<std::uint8_t> append_byte(
    const std::array<std::uint8_t, 3>& prefix,
    std::uint8_t suffix) {
    return {prefix[0], prefix[1], prefix[2], suffix};
}

struct LeftCandidate {
    std::uint8_t suffix = 0U;
    pvc::InternalStateSnapshot state{};
};

struct Counts {
    std::uint64_t pairs = 0U;
    std::uint64_t cross_pairs = 0U;
    std::uint64_t fingerprint_candidates = 0U;
    std::uint64_t exact_merges = 0U;
    std::uint64_t inherited_merges = 0U;
    std::uint64_t local_merges = 0U;
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const auto pairs = pvc::tool::all_three_byte_forward_collisions(
            preset.parameters, options.prefix_count, options.threads);
        const auto thread_count = std::min(options.threads,
                                           std::max<std::size_t>(1U, pairs.size()));

        std::cout << "Independent one-byte suffix catalog search\n"
                  << "preset=" << preset.name
                  << " forward_collision_pairs=" << pairs.size()
                  << " suffixes_per_side=256"
                  << " cross_space_per_pair=65536\n";
        pvc::tool::print_parameters(preset.parameters);

        std::atomic<std::size_t> next_pair{0U};
        std::vector<Counts> per_thread(thread_count);
        std::vector<std::thread> workers;
        workers.reserve(thread_count);

        for (std::size_t worker_index = 0U;
             worker_index < thread_count;
             ++worker_index) {
            workers.emplace_back([&, worker_index] {
                auto& counts = per_thread[worker_index];
                while (true) {
                    const auto pair_index = next_pair.fetch_add(
                        1U, std::memory_order_relaxed);
                    if (pair_index >= pairs.size()) {
                        break;
                    }
                    const auto& pair = pairs[pair_index];
                    ++counts.pairs;
                    counts.cross_pairs += 65536U;

                    std::unordered_map<pvc::tool::StateFingerprint,
                                       std::vector<LeftCandidate>,
                                       pvc::tool::StateFingerprintHash> left_table;
                    left_table.reserve(512U);
                    for (std::uint16_t suffix = 0U; suffix < 256U; ++suffix) {
                        const auto message = append_byte(
                            pair.left, static_cast<std::uint8_t>(suffix));
                        auto state = pvc::foldback_state_for_research(
                            message, preset.parameters);
                        left_table[pvc::tool::analysis_fingerprint(state)].push_back(
                            LeftCandidate{
                                .suffix = static_cast<std::uint8_t>(suffix),
                                .state = std::move(state),
                            });
                    }

                    for (std::uint16_t suffix = 0U; suffix < 256U; ++suffix) {
                        const auto message = append_byte(
                            pair.right, static_cast<std::uint8_t>(suffix));
                        const auto state = pvc::foldback_state_for_research(
                            message, preset.parameters);
                        const auto found = left_table.find(
                            pvc::tool::analysis_fingerprint(state));
                        if (found == left_table.end()) {
                            continue;
                        }
                        counts.fingerprint_candidates += found->second.size();
                        for (const auto& candidate : found->second) {
                            if (candidate.state != state) {
                                continue;
                            }
                            ++counts.exact_merges;
                            if (pair.inherited) {
                                ++counts.inherited_merges;
                            } else {
                                ++counts.local_merges;
                            }
                        }
                    }
                }
            });
        }
        for (auto& worker : workers) {
            worker.join();
        }

        Counts total;
        for (const auto& counts : per_thread) {
            total.pairs += counts.pairs;
            total.cross_pairs += counts.cross_pairs;
            total.fingerprint_candidates += counts.fingerprint_candidates;
            total.exact_merges += counts.exact_merges;
            total.inherited_merges += counts.inherited_merges;
            total.local_merges += counts.local_merges;
        }

        std::cout << "processed_forward_collision_pairs=" << total.pairs << '\n'
                  << "logical_cross_suffix_pairs=" << total.cross_pairs << '\n'
                  << "fingerprint_candidate_pairs="
                  << total.fingerprint_candidates << '\n'
                  << "exact_after_foldback_merges=" << total.exact_merges << '\n'
                  << "inherited_pair_merges=" << total.inherited_merges << '\n'
                  << "local_pair_merges=" << total.local_merges << '\n';
        if (total.exact_merges == 0U) {
            std::cout << "No independent one-byte suffix produced an equal-length "
                         "after-foldback state merge.\n";
        }
        return total.exact_merges == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
