#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::size_t samples = 256;
    std::size_t message_bytes = 16;
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
        } else {
            throw std::invalid_argument(
                "usage: pvc-related-input-probe [--preset NAME] "
                "[--samples N] [--message-bytes N>=16]");
        }
    }
    if (options.message_bytes < 16U) {
        throw std::invalid_argument("message-bytes must be at least 16");
    }
    return options;
}

struct RelationMetrics {
    std::string name;
    std::uint64_t bit_sum = 0;
    std::size_t count = 0;
    std::size_t digest_matches = 0;
    std::size_t state_matches = 0;
};

void compare_relation(RelationMetrics& metrics,
                      const std::vector<std::uint8_t>& base,
                      const std::vector<std::uint8_t>& related,
                      const pvc::HashParameters& parameters) {
    const auto left = pvc::inspect_with_parameters(base, parameters, false, false);
    const auto right = pvc::inspect_with_parameters(related, parameters, false, false);
    metrics.bit_sum += pvc::tool::byte_hamming_distance(left.digest, right.digest);
    ++metrics.count;
    if (left.digest == right.digest) {
        ++metrics.digest_matches;
    }
    if (left.final_state == right.final_state) {
        ++metrics.state_matches;
    }
}

void run_permutation_domain(std::vector<std::uint8_t> message,
                            std::string_view label,
                            const pvc::HashParameters& parameters) {
    std::sort(message.begin(), message.end());
    std::map<std::vector<std::uint8_t>, std::vector<std::uint8_t>, pvc::tool::StateKeyLess> digests;
    std::map<std::vector<std::uint8_t>, std::vector<std::uint8_t>, pvc::tool::StateKeyLess> states;
    std::size_t count = 0;
    std::size_t digest_collisions = 0;
    std::size_t state_collisions = 0;

    do {
        const auto result = pvc::inspect_with_parameters(message, parameters, false, false);
        if (!digests.emplace(result.digest, message).second) {
            ++digest_collisions;
        }
        if (!states.emplace(pvc::tool::serialize_state(result.final_state), message).second) {
            ++state_collisions;
        }
        ++count;
    } while (std::next_permutation(message.begin(), message.end()));

    std::cout << "permutation_domain=" << label
              << " unique_messages=" << count
              << " digest_collisions=" << digest_collisions
              << " state_collisions=" << state_collisions << '\n';
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);

        std::cout << "Related-input and multiset probe\n"
                  << "preset=" << preset.name
                  << " samples=" << options.samples
                  << " message_bytes=" << options.message_bytes << '\n';
        pvc::tool::print_parameters(preset.parameters);

        RelationMetrics reversed{"reverse"};
        RelationMetrics complemented{"complement"};
        RelationMetrics rotated{"rotate-left-one"};

        for (std::size_t sample = 0; sample < options.samples; ++sample) {
            const auto base = pvc::tool::counter_message(
                static_cast<std::uint64_t>(sample), 0x52454C415445442DULL,
                options.message_bytes);

            auto reverse = base;
            std::reverse(reverse.begin(), reverse.end());
            compare_relation(reversed, base, reverse, preset.parameters);

            auto complement = base;
            for (auto& byte : complement) {
                byte = static_cast<std::uint8_t>(byte ^ 0xFFU);
            }
            compare_relation(complemented, base, complement, preset.parameters);

            auto rotation = base;
            std::rotate(rotation.begin(), rotation.begin() + 1, rotation.end());
            compare_relation(rotated, base, rotation, preset.parameters);
        }

        for (const auto& metrics : {reversed, complemented, rotated}) {
            std::cout << "relation=" << metrics.name
                      << " mean_digest_bits=" << std::fixed << std::setprecision(4)
                      << (static_cast<double>(metrics.bit_sum)
                          / static_cast<double>(metrics.count))
                      << " digest_matches=" << metrics.digest_matches
                      << " state_matches=" << metrics.state_matches << '\n';
        }

        run_permutation_domain(
            std::vector<std::uint8_t>{'A','B','C','D','E','F','G','H'},
            "ABCDEFGH",
            preset.parameters);
        run_permutation_domain(
            std::vector<std::uint8_t>{'A','A','B','B','C','C','D','D'},
            "AABBCCDD",
            preset.parameters);
        run_permutation_domain(
            std::vector<std::uint8_t>{0,0,0,0,1,1,1,1},
            "00001111",
            preset.parameters);

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
