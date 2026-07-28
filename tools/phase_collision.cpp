#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t message_bytes = 2;
    std::size_t print_limit = 1;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--message-bytes" && i + 1 < argc) {
            options.message_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--print-limit" && i + 1 < argc) {
            options.print_limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-phase-collision [--preset NAME] [--message-bytes 1|2] "
                "[--print-limit N]");
        }
    }
    if (options.message_bytes == 0U || options.message_bytes > 2U) {
        throw std::invalid_argument("message-bytes must be 1 or 2");
    }
    return options;
}

std::vector<std::uint8_t> message_from_counter(std::uint32_t counter,
                                               std::size_t bytes) {
    std::vector<std::uint8_t> message(bytes);
    for (std::size_t i = 0; i < bytes; ++i) {
        const auto shift = static_cast<unsigned>((bytes - 1U - i) * 8U);
        message[i] = static_cast<std::uint8_t>((counter >> shift) & 0xFFU);
    }
    return message;
}

const pvc::InternalStateSnapshot& checkpoint_state(
    const pvc::ResearchHashResult& result,
    pvc::ResearchPhase phase) {
    if (phase == pvc::ResearchPhase::Final) {
        return result.final_state;
    }
    for (const auto& checkpoint : result.checkpoints) {
        if (checkpoint.phase == phase) {
            return checkpoint.state;
        }
    }
    throw std::logic_error("requested phase checkpoint is missing");
}

struct FingerprintBucket {
    std::vector<std::uint32_t> counters;
};

struct PhaseTable {
    pvc::ResearchPhase phase = pvc::ResearchPhase::Initial;
    std::unordered_map<pvc::tool::StateFingerprint,
                       FingerprintBucket,
                       pvc::tool::StateFingerprintHash> seen;
    std::size_t collisions = 0;
    std::size_t printed = 0;
};

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        const std::uint32_t domain = options.message_bytes == 1U ? 256U : 65536U;

        std::cout << "Phase collision enumeration\n"
                  << "preset=" << preset.name
                  << " message_bytes=" << options.message_bytes
                  << " domain=" << domain << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::vector<PhaseTable> tables;
        for (const auto phase : {
                 pvc::ResearchPhase::AfterForward,
                 pvc::ResearchPhase::AfterFoldback,
                 pvc::ResearchPhase::AfterDiagonalClosure,
                 pvc::ResearchPhase::AfterOrbitClosure,
                 pvc::ResearchPhase::Final}) {
            PhaseTable table;
            table.phase = phase;
            table.seen.reserve(domain);
            tables.push_back(std::move(table));
        }

        std::map<std::vector<std::uint8_t>, std::uint32_t,
                 pvc::tool::StateKeyLess> digest_seen;
        std::size_t digest_collisions = 0;
        std::size_t digest_printed = 0;

        for (std::uint32_t counter = 0; counter < domain; ++counter) {
            const auto message = message_from_counter(counter, options.message_bytes);
            const auto result = pvc::inspect_with_parameters(
                message, preset.parameters, false, true);

            for (auto& table : tables) {
                const auto& state = checkpoint_state(result, table.phase);
                const auto fingerprint = pvc::tool::analysis_fingerprint(state);
                auto& bucket = table.seen[fingerprint].counters;

                bool exact_collision = false;
                std::uint32_t first_counter = 0;
                for (const auto candidate : bucket) {
                    const auto candidate_message = message_from_counter(
                        candidate, options.message_bytes);
                    const auto candidate_result = pvc::inspect_with_parameters(
                        candidate_message, preset.parameters, false, true);
                    const auto& candidate_state = checkpoint_state(
                        candidate_result, table.phase);
                    if (candidate_state == state) {
                        exact_collision = true;
                        first_counter = candidate;
                        break;
                    }
                }

                if (exact_collision) {
                    ++table.collisions;
                    if (table.printed < options.print_limit) {
                        const auto first_message = message_from_counter(
                            first_counter, options.message_bytes);
                        std::cout << "first_state_collision phase="
                                  << pvc::research_phase_name(table.phase)
                                  << " a=" << pvc::tool::hex_bytes(first_message)
                                  << " b=" << pvc::tool::hex_bytes(message) << '\n';
                        ++table.printed;
                    }
                }
                bucket.push_back(counter);
            }

            const auto [digest_it, digest_inserted] = digest_seen.emplace(
                result.digest, counter);
            if (!digest_inserted) {
                ++digest_collisions;
                if (digest_printed < options.print_limit) {
                    const auto first_message = message_from_counter(
                        digest_it->second, options.message_bytes);
                    std::cout << "first_digest_collision a="
                              << pvc::tool::hex_bytes(first_message)
                              << " b=" << pvc::tool::hex_bytes(message) << '\n';
                    ++digest_printed;
                }
            }
        }

        std::cout << "\nphase,collisions,distinct_fingerprints\n";
        for (const auto& table : tables) {
            std::cout << pvc::research_phase_name(table.phase) << ','
                      << table.collisions << ','
                      << table.seen.size() << '\n';
        }
        std::cout << "digest_collisions=" << digest_collisions
                  << " distinct_digests=" << digest_seen.size() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
