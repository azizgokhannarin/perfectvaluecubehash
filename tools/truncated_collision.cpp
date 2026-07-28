#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

struct Options {
    std::string preset = "R3-medium";
    unsigned bits = 32;
    std::uint64_t limit = 500000;
    std::size_t message_bytes = 16;
    std::uint64_t trial = 0;
};

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--bits" && i + 1 < argc) {
            options.bits = static_cast<unsigned>(std::stoul(argv[++i]));
        } else if (arg == "--limit" && i + 1 < argc) {
            options.limit = std::stoull(argv[++i]);
        } else if (arg == "--message-bytes" && i + 1 < argc) {
            options.message_bytes = static_cast<std::size_t>(std::stoul(argv[++i]));
        } else if (arg == "--trial" && i + 1 < argc) {
            options.trial = std::stoull(argv[++i]);
        } else {
            throw std::invalid_argument(
                "usage: pvc-truncated-collision [--preset NAME] [--bits 1..64] "
                "[--limit N] [--message-bytes N>=16] [--trial N]");
        }
    }
    if (options.bits == 0U || options.bits > 64U) {
        throw std::invalid_argument("bits must be in [1,64]");
    }
    if (options.message_bytes < 16U) {
        throw std::invalid_argument("message-bytes must be at least 16");
    }
    return options;
}

std::uint64_t truncate_digest(const std::vector<std::uint8_t>& digest,
                              unsigned bits) {
    if (digest.size() * 8U < bits) {
        throw std::invalid_argument("digest is shorter than requested truncation");
    }
    std::uint64_t value = 0;
    for (unsigned bit = 0; bit < bits; ++bit) {
        const auto byte_index = static_cast<std::size_t>(bit / 8U);
        const auto bit_index = 7U - (bit & 7U);
        const auto selected = static_cast<std::uint64_t>(
            (digest[byte_index] >> bit_index) & 1U);
        value = (value << 1U) | selected;
    }
    return value;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        if (preset.parameters.squeeze_bytes * 8U < options.bits) {
            throw std::invalid_argument("selected preset digest is too short for --bits");
        }

        std::cout << "Truncated collision scaling probe\n"
                  << "preset=" << preset.name
                  << " bits=" << options.bits
                  << " limit=" << options.limit
                  << " message_bytes=" << options.message_bytes
                  << " trial=" << options.trial << '\n';
        pvc::tool::print_parameters(preset.parameters);

        std::unordered_map<std::uint64_t, std::uint64_t> seen;
        if (options.limit < static_cast<std::uint64_t>(
                std::numeric_limits<std::size_t>::max())) {
            seen.reserve(static_cast<std::size_t>(options.limit));
        }

        for (std::uint64_t counter = 0; counter < options.limit; ++counter) {
            const auto message = pvc::tool::counter_message(
                counter, options.trial, options.message_bytes);
            const auto digest = pvc::hash_with_parameters(message, preset.parameters);
            const auto truncated = truncate_digest(digest, options.bits);
            const auto [it, inserted] = seen.emplace(truncated, counter);
            if (!inserted) {
                const auto first_message = pvc::tool::counter_message(
                    it->second, options.trial, options.message_bytes);
                const auto first_digest = pvc::hash_with_parameters(
                    first_message, preset.parameters);
                const auto expected = std::sqrt(3.14159265358979323846 / 2.0)
                                    * std::pow(2.0,
                                               static_cast<double>(options.bits) / 2.0);
                const auto observed = static_cast<double>(counter + 1U);

                std::cout << "collision found\n"
                          << "first_counter=" << it->second << '\n'
                          << "second_counter=" << counter << '\n'
                          << "truncated=0x" << std::hex << truncated << std::dec << '\n'
                          << "messages_tested=" << (counter + 1U) << '\n'
                          << "birthday_first-collision_expectation="
                          << std::fixed << std::setprecision(2) << expected << '\n'
                          << "observed/expected=" << (observed / expected) << '\n'
                          << "full_digest_equal="
                          << (first_digest == digest ? "yes" : "no") << '\n'
                          << "a=" << pvc::tool::hex_bytes(first_message) << '\n'
                          << "b=" << pvc::tool::hex_bytes(message) << '\n';
                return 0;
            }
        }

        const auto expected = std::sqrt(3.14159265358979323846 / 2.0)
                            * std::pow(2.0,
                                       static_cast<double>(options.bits) / 2.0);
        std::cout << "No collision within limit.\n"
                  << "birthday_first-collision_expectation="
                  << std::fixed << std::setprecision(2) << expected << '\n'
                  << "limit/expected="
                  << (static_cast<double>(options.limit) / expected) << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
