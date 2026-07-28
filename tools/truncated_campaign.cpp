#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace {

struct Options {
    std::string preset = "R5-canonical";
    std::vector<unsigned> bits{24U, 32U, 40U, 48U};
    std::size_t trials = 8U;
    std::uint64_t limit = 500000U;
    std::size_t message_bytes = 16U;
};

std::vector<unsigned> parse_bits(std::string_view text) {
    std::vector<unsigned> result;
    std::stringstream stream{std::string(text)};
    std::string token;
    while (std::getline(stream, token, ',')) {
        const auto value = static_cast<unsigned>(std::stoul(token));
        if (value == 0U || value > 64U) {
            throw std::invalid_argument("each bit width must be in [1,64]");
        }
        result.push_back(value);
    }
    if (result.empty()) {
        throw std::invalid_argument("bits list cannot be empty");
    }
    return result;
}

Options parse_options(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg = argv[i];
        if (arg == "--preset" && i + 1 < argc) {
            options.preset = argv[++i];
        } else if (arg == "--bits" && i + 1 < argc) {
            options.bits = parse_bits(argv[++i]);
        } else if (arg == "--trials" && i + 1 < argc) {
            options.trials = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--limit" && i + 1 < argc) {
            options.limit = std::stoull(argv[++i]);
        } else if (arg == "--message-bytes" && i + 1 < argc) {
            options.message_bytes = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else {
            throw std::invalid_argument(
                "usage: pvc-truncated-campaign [--preset NAME] "
                "[--bits 24,32,40,48] [--trials N] [--limit N] "
                "[--message-bytes N>=16]");
        }
    }
    if (options.trials == 0U || options.trials > 1000U) {
        throw std::invalid_argument("trials must be in [1,1000]");
    }
    if (options.limit == 0U) {
        throw std::invalid_argument("limit must be nonzero");
    }
    if (options.message_bytes < 16U) {
        throw std::invalid_argument("message-bytes must be at least 16");
    }
    return options;
}

std::uint64_t truncate_digest(const std::vector<std::uint8_t>& digest,
                              unsigned bits) {
    std::uint64_t value = 0U;
    for (unsigned bit = 0U; bit < bits; ++bit) {
        const auto byte_index = static_cast<std::size_t>(bit / 8U);
        const auto bit_index = 7U - (bit & 7U);
        value = (value << 1U)
              | static_cast<std::uint64_t>(
                    (digest[byte_index] >> bit_index) & 1U);
    }
    return value;
}

struct TrialResult {
    bool found = false;
    std::uint64_t messages = 0U;
};

TrialResult run_trial(const pvc::HashParameters& parameters,
                      unsigned bits,
                      std::uint64_t limit,
                      std::size_t message_bytes,
                      std::uint64_t domain) {
    std::unordered_map<std::uint64_t, std::uint64_t> seen;
    if (limit <= static_cast<std::uint64_t>(
            std::numeric_limits<std::size_t>::max())) {
        seen.reserve(static_cast<std::size_t>(limit));
    }
    for (std::uint64_t counter = 0U; counter < limit; ++counter) {
        const auto message = pvc::tool::counter_message(
            counter, domain, message_bytes);
        const auto digest = pvc::hash_with_parameters(message, parameters);
        const auto value = truncate_digest(digest, bits);
        if (!seen.emplace(value, counter).second) {
            return TrialResult{.found = true, .messages = counter + 1U};
        }
    }
    return TrialResult{.found = false, .messages = limit};
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = parse_options(argc, argv);
        const auto& preset = pvc::tool::find_preset(options.preset);
        std::cout << "Truncated collision multi-width campaign\n"
                  << "preset=" << preset.name
                  << " trials=" << options.trials
                  << " limit_per_trial=" << options.limit
                  << " message_bytes=" << options.message_bytes << '\n';
        pvc::tool::print_parameters(preset.parameters);
        std::cout << "bits,found,censored,mean_messages,expected,mean_ratio,min,max\n";

        for (const auto bits : options.bits) {
            if (bits > preset.parameters.squeeze_bytes * 8U) {
                throw std::invalid_argument("bit width exceeds preset digest size");
            }
            std::size_t found = 0U;
            std::uint64_t total = 0U;
            std::uint64_t minimum = std::numeric_limits<std::uint64_t>::max();
            std::uint64_t maximum = 0U;
            for (std::size_t trial = 0U; trial < options.trials; ++trial) {
                const auto result = run_trial(
                    preset.parameters, bits, options.limit,
                    options.message_bytes,
                    static_cast<std::uint64_t>(trial)
                        ^ (static_cast<std::uint64_t>(bits) << 32U));
                if (result.found) {
                    ++found;
                    total += result.messages;
                    minimum = std::min(minimum, result.messages);
                    maximum = std::max(maximum, result.messages);
                }
            }
            const auto expected = std::sqrt(3.14159265358979323846 / 2.0)
                                * std::pow(2.0, static_cast<double>(bits) / 2.0);
            const auto mean = found == 0U
                ? 0.0
                : static_cast<double>(total) / static_cast<double>(found);
            std::cout << bits << ','
                      << found << ','
                      << (options.trials - found) << ','
                      << std::fixed << std::setprecision(4) << mean << ','
                      << expected << ','
                      << (found == 0U ? 0.0 : mean / expected) << ',';
            if (found == 0U) {
                std::cout << "NA,NA\n";
            } else {
                std::cout << minimum << ',' << maximum << '\n';
            }
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
