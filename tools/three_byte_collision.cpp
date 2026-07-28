#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t first_byte_count = 256;
    std::size_t threads = 0;
    std::size_t print_limit = 12;
    std::string phase = "forward";
};

struct Entry {
    std::uint64_t fingerprint = 0;
    std::uint32_t message = 0;
};

static_assert(sizeof(Entry) <= 16U);

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--first-byte-count" && i + 1 < argc) {
            options.first_byte_count = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--threads" && i + 1 < argc) {
            options.threads = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--phase" && i + 1 < argc) {
            options.phase = argv[++i];
        } else {
            throw std::invalid_argument(
                "usage: pvc-three-byte-collision [--preset NAME] "
                "[--first-byte-count 1..256] [--threads N] [--print-limit N] "
                "[--phase forward|foldback]");
        }
    }
    if (options.first_byte_count == 0U || options.first_byte_count > 256U) {
        throw std::invalid_argument("first-byte-count must be in [1,256]");
    }
    if (options.phase != "forward" && options.phase != "foldback") {
        throw std::invalid_argument("phase must be forward or foldback");
    }
    if (options.threads == 0U) {
        options.threads = std::max<std::size_t>(1U, std::thread::hardware_concurrency());
    }
    options.threads = std::min(options.threads, options.first_byte_count);
    return options;
}

std::array<std::uint8_t, 3> decode_message(std::uint32_t value) {
    return {
        static_cast<std::uint8_t>((value >> 16U) & 0xFFU),
        static_cast<std::uint8_t>((value >> 8U) & 0xFFU),
        static_cast<std::uint8_t>(value & 0xFFU),
    };
}

std::vector<std::uint8_t> vector_message(std::uint32_t value) {
    const auto bytes = decode_message(value);
    return {bytes.begin(), bytes.end()};
}

std::string message_hex(std::uint32_t value) {
    return pvc::tool::hex_bytes(vector_message(value));
}

pvc::InternalStateSnapshot scanned_state(std::uint32_t value,
                                         const pvc::HashParameters& parameters,
                                         std::string_view phase) {
    const auto bytes = decode_message(value);
    if (phase == "foldback") {
        return pvc::foldback_state_for_research(bytes, parameters);
    }
    return pvc::forward_state_for_research(bytes, parameters);
}

pvc::InternalStateSnapshot prefix_state(std::uint32_t value,
                                        const pvc::HashParameters& parameters) {
    const auto bytes = decode_message(value);
    const std::array<std::uint8_t, 2> prefix{bytes[0], bytes[1]};
    return pvc::forward_state_for_research(prefix, parameters);
}

bool same_physical_trace(const std::vector<pvc::Move>& left,
                         const std::vector<pvc::Move>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (left[i].axis != right[i].axis
            || left[i].intersection_before != right[i].intersection_before
            || left[i].intersection_after != right[i].intersection_after
            || left[i].amount != right[i].amount
            || left[i].phase != right[i].phase
            || left[i].symbol_index != right[i].symbol_index) {
            return false;
        }
    }
    return true;
}

void radix_sort_by_fingerprint(std::vector<Entry>& entries) {
    std::vector<Entry> scratch(entries.size());
    auto* source = &entries;
    auto* target = &scratch;

    for (unsigned pass = 0; pass < 4U; ++pass) {
        constexpr std::size_t buckets = 1U << 16U;
        std::array<std::size_t, buckets> counts{};
        const unsigned shift = pass * 16U;

        for (const auto& entry : *source) {
            ++counts[(entry.fingerprint >> shift) & 0xFFFFULL];
        }

        std::size_t offset = 0;
        for (auto& count : counts) {
            const auto current = count;
            count = offset;
            offset += current;
        }

        for (const auto& entry : *source) {
            const auto bucket = static_cast<std::size_t>(
                (entry.fingerprint >> shift) & 0xFFFFULL);
            (*target)[counts[bucket]++] = entry;
        }
        std::swap(source, target);
    }

    if (source != &entries) {
        entries.swap(*source);
    }
}

struct CollisionSummary {
    std::uint64_t exact_groups = 0;
    std::uint64_t duplicate_messages = 0;
    std::uint64_t exact_pairs = 0;
    std::uint64_t inherited_pairs = 0;
    std::uint64_t new_pairs = 0;
    std::uint64_t delta42_pairs = 0;
    std::uint64_t local_third_symbol_pairs = 0;
    std::uint64_t local_identical_move_paths = 0;
    std::uint64_t local_distinct_move_paths = 0;
    std::uint64_t multi_position_pairs = 0;
    std::array<std::uint64_t, 256> local_third_absolute_deltas{};
    std::uint64_t foldback_pairs = 0;
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const std::size_t domain = options.first_byte_count * 65536ULL;

        std::cout << "Three-byte exact state collision scan\n"
                  << "phase=" << options.phase << '\n'
                  << "preset=" << preset.name
                  << " first_byte_count=" << options.first_byte_count
                  << " domain=" << domain
                  << " threads=" << options.threads << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<Entry> entries(domain);
        std::array<pvc::InternalStateSnapshot, 256> first_states{};
        const auto initial = pvc::initial_internal_state();
        for (std::size_t first = 0; first < options.first_byte_count; ++first) {
            first_states[first] = pvc::absorb_symbol_for_research(
                initial, static_cast<std::uint8_t>(first), preset.parameters);
        }

        const auto generation_start = std::chrono::steady_clock::now();
        std::atomic<std::size_t> next_first{0};
        std::vector<std::thread> workers;
        workers.reserve(options.threads);

        for (std::size_t worker = 0; worker < options.threads; ++worker) {
            workers.emplace_back([&] {
                while (true) {
                    const auto first = next_first.fetch_add(1U);
                    if (first >= options.first_byte_count) {
                        return;
                    }
                    for (std::size_t second = 0; second < 256U; ++second) {
                        const auto state_two = pvc::absorb_symbol_for_research(
                            first_states[first],
                            static_cast<std::uint8_t>(second),
                            preset.parameters);
                        for (std::size_t third = 0; third < 256U; ++third) {
                            const auto message = static_cast<std::uint32_t>(
                                (first << 16U) | (second << 8U) | third);
                            pvc::InternalStateSnapshot state_three;
                            if (options.phase == "forward") {
                                state_three = pvc::absorb_symbol_for_research(
                                    state_two,
                                    static_cast<std::uint8_t>(third),
                                    preset.parameters);
                            } else {
                                state_three = scanned_state(
                                    message, preset.parameters, options.phase);
                            }
                            const auto index = first * 65536U + second * 256U + third;
                            entries[index] = Entry{
                                .fingerprint = pvc::tool::fast_state_fingerprint64(state_three),
                                .message = message,
                            };
                        }
                    }
                }
            });
        }
        for (auto& worker : workers) {
            worker.join();
        }
        const auto generation_end = std::chrono::steady_clock::now();

        const auto sort_start = std::chrono::steady_clock::now();
        radix_sort_by_fingerprint(entries);
        const auto sort_end = std::chrono::steady_clock::now();

        CollisionSummary summary;
        std::size_t printed = 0;
        std::size_t fingerprint_buckets = 0;
        std::size_t fingerprint_candidates = 0;

        for (std::size_t begin = 0; begin < entries.size();) {
            std::size_t end = begin + 1U;
            while (end < entries.size()
                   && entries[end].fingerprint == entries[begin].fingerprint) {
                ++end;
            }
            if (end - begin > 1U) {
                ++fingerprint_buckets;
                fingerprint_candidates += end - begin;

                std::vector<pvc::InternalStateSnapshot> states;
                states.reserve(end - begin);
                for (std::size_t i = begin; i < end; ++i) {
                    states.push_back(scanned_state(entries[i].message, preset.parameters, options.phase));
                }

                std::vector<bool> assigned(states.size(), false);
                for (std::size_t i = 0; i < states.size(); ++i) {
                    if (assigned[i]) {
                        continue;
                    }
                    std::vector<std::size_t> group{i};
                    assigned[i] = true;
                    for (std::size_t j = i + 1U; j < states.size(); ++j) {
                        if (!assigned[j] && states[j] == states[i]) {
                            assigned[j] = true;
                            group.push_back(j);
                        }
                    }
                    if (group.size() < 2U) {
                        continue;
                    }

                    ++summary.exact_groups;
                    summary.duplicate_messages += group.size() - 1U;
                    summary.exact_pairs += group.size() * (group.size() - 1U) / 2U;

                    for (std::size_t a = 0; a < group.size(); ++a) {
                        for (std::size_t b = a + 1U; b < group.size(); ++b) {
                            const auto left_value = entries[begin + group[a]].message;
                            const auto right_value = entries[begin + group[b]].message;
                            const auto left = decode_message(left_value);
                            const auto right = decode_message(right_value);
                            const bool common_suffix = left[2] == right[2];
                            bool inherited = false;
                            bool local_third = false;
                            int delta_second = 0;

                            if (options.phase == "forward") {
                                inherited = common_suffix
                                    && prefix_state(left_value, preset.parameters)
                                        == prefix_state(right_value, preset.parameters);
                                if (inherited) {
                                    ++summary.inherited_pairs;
                                } else {
                                    ++summary.new_pairs;
                                }

                                delta_second = static_cast<int>(right[1])
                                             - static_cast<int>(left[1]);
                                if (left[0] == right[0] && common_suffix
                                    && (delta_second == 42 || delta_second == -42)) {
                                    ++summary.delta42_pairs;
                                }

                                std::size_t differing_positions = 0;
                                for (std::size_t position = 0; position < 3U; ++position) {
                                    if (left[position] != right[position]) {
                                        ++differing_positions;
                                    }
                                }
                                local_third = left[0] == right[0]
                                    && left[1] == right[1]
                                    && left[2] != right[2];
                                if (local_third) {
                                    ++summary.local_third_symbol_pairs;
                                    const auto delta = static_cast<std::size_t>(
                                        std::abs(static_cast<int>(right[2])
                                               - static_cast<int>(left[2])));
                                    ++summary.local_third_absolute_deltas[delta];

                                    const auto context = prefix_state(
                                        left_value, preset.parameters);
                                    std::vector<pvc::Move> left_trace;
                                    std::vector<pvc::Move> right_trace;
                                    static_cast<void>(pvc::absorb_symbol_for_research(
                                        context, left[2], preset.parameters, &left_trace));
                                    static_cast<void>(pvc::absorb_symbol_for_research(
                                        context, right[2], preset.parameters, &right_trace));
                                    if (same_physical_trace(left_trace, right_trace)) {
                                        ++summary.local_identical_move_paths;
                                    } else {
                                        ++summary.local_distinct_move_paths;
                                    }
                                } else if (differing_positions > 1U) {
                                    ++summary.multi_position_pairs;
                                }
                            }

                            const auto left_message = vector_message(left_value);
                            const auto right_message = vector_message(right_value);
                            bool foldback_equal = options.phase == "foldback";
                            if (options.phase == "forward") {
                                const auto left_foldback = pvc::foldback_state_for_research(
                                    left_message, preset.parameters);
                                const auto right_foldback = pvc::foldback_state_for_research(
                                    right_message, preset.parameters);
                                foldback_equal = left_foldback == right_foldback;
                            }
                            if (foldback_equal) {
                                ++summary.foldback_pairs;
                            }

                            if (printed < options.print_limit) {
                                std::cout << "collision[" << printed << "]"
                                          << " a=" << message_hex(left_value)
                                          << " b=" << message_hex(right_value)
                                          << " inherited=" << (inherited ? "yes" : "no")
                                          << " delta42="
                                          << ((left[0] == right[0] && common_suffix
                                               && (delta_second == 42 || delta_second == -42))
                                                  ? "yes" : "no")
                                          << " local_third="
                                          << (local_third ? "yes" : "no")
                                          << " after_foldback_equal="
                                          << (foldback_equal ? "yes" : "no") << '\n';
                                ++printed;
                            }
                        }
                    }
                }
            }
            begin = end;
        }

        const auto finish = std::chrono::steady_clock::now();
        const auto generation_seconds = std::chrono::duration<double>(
            generation_end - generation_start).count();
        const auto sort_seconds = std::chrono::duration<double>(sort_end - sort_start).count();
        const auto verify_seconds = std::chrono::duration<double>(finish - sort_end).count();

        std::cout << "\nmetric,value\n"
                  << "phase," << options.phase << '\n'
                  << "domain," << domain << '\n'
                  << "fingerprint_buckets," << fingerprint_buckets << '\n'
                  << "fingerprint_candidate_messages," << fingerprint_candidates << '\n'
                  << "exact_state_groups," << summary.exact_groups << '\n'
                  << "duplicate_messages," << summary.duplicate_messages << '\n'
                  << "exact_state_pairs," << summary.exact_pairs << '\n'
                  << "inherited_pairs," << summary.inherited_pairs << '\n'
                  << "new_convergence_pairs," << summary.new_pairs << '\n'
                  << "delta42_pairs," << summary.delta42_pairs << '\n'
                  << "local_third_symbol_pairs," << summary.local_third_symbol_pairs << '\n'
                  << "local_identical_move_paths," << summary.local_identical_move_paths << '\n'
                  << "local_distinct_move_paths," << summary.local_distinct_move_paths << '\n'
                  << "multi_position_pairs," << summary.multi_position_pairs << '\n'
                  << "after_foldback_pairs," << summary.foldback_pairs << '\n';
        std::cout << "local_third_absolute_delta,hits\n";
        for (std::size_t delta = 1; delta < summary.local_third_absolute_deltas.size(); ++delta) {
            if (summary.local_third_absolute_deltas[delta] != 0U) {
                std::cout << delta << ',' << summary.local_third_absolute_deltas[delta] << '\n';
            }
        }
        std::cout
                  << std::fixed << std::setprecision(3)
                  << "generation_seconds," << generation_seconds << '\n'
                  << "radix_sort_seconds," << sort_seconds << '\n'
                  << "verification_seconds," << verify_seconds << '\n';

        if (options.phase == "foldback") {
            return summary.exact_pairs == 0U ? 0 : 1;
        }
        return summary.foldback_pairs == 0U ? 0 : 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
