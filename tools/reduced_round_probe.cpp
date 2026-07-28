#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <vector>

namespace {

struct Metrics {
    std::size_t digest_collisions = 0;
    std::size_t state_collisions = 0;
    double avalanche_mean = 0.0;
    std::size_t avalanche_min = std::numeric_limits<std::size_t>::max();
    std::size_t avalanche_max = 0;
};

Metrics evaluate(const pvc::HashParameters& parameters) {
    Metrics metrics;
    std::map<std::vector<std::uint8_t>, std::uint16_t, pvc::tool::StateKeyLess> digests;
    std::map<std::vector<std::uint8_t>, std::uint16_t, pvc::tool::StateKeyLess> states;

    std::array<std::vector<std::uint8_t>, 256> digest_cache{};

    for (unsigned value = 0; value < 256U; ++value) {
        const std::array<std::uint8_t, 1> message{
            static_cast<std::uint8_t>(value)
        };
        const auto result = pvc::inspect_with_parameters(message, parameters, false, false);
        digest_cache[value] = result.digest;

        if (!digests.emplace(result.digest, static_cast<std::uint16_t>(value)).second) {
            ++metrics.digest_collisions;
        }
        if (!states.emplace(pvc::tool::serialize_state(result.final_state),
                            static_cast<std::uint16_t>(value)).second) {
            ++metrics.state_collisions;
        }
    }

    std::uint64_t avalanche_sum = 0;
    std::size_t avalanche_count = 0;
    for (unsigned value = 0; value < 256U; ++value) {
        for (unsigned bit = 0; bit < 8U; ++bit) {
            const auto changed = value ^ (1U << bit);
            const auto distance = pvc::tool::byte_hamming_distance(
                digest_cache[value], digest_cache[changed]);
            avalanche_sum += distance;
            ++avalanche_count;
            metrics.avalanche_min = std::min(metrics.avalanche_min, distance);
            metrics.avalanche_max = std::max(metrics.avalanche_max, distance);
        }
    }
    metrics.avalanche_mean = static_cast<double>(avalanche_sum)
                           / static_cast<double>(avalanche_count);
    return metrics;
}

} // namespace

int main() {
    try {
        std::cout << "PVC reduced-round matrix\n";
        std::cout << "Each row exhausts all 256 one-byte messages.\n\n";
        std::cout << std::left
                  << std::setw(15) << "preset"
                  << std::setw(9) << "digestB"
                  << std::setw(12) << "digest-col"
                  << std::setw(11) << "state-col"
                  << std::setw(12) << "avalanche"
                  << std::setw(8) << "min"
                  << std::setw(8) << "max"
                  << '\n';

        for (const auto& preset : pvc::reduced_round_presets()) {
            const auto metrics = evaluate(preset.parameters);
            std::cout << std::left
                      << std::setw(15) << preset.name
                      << std::setw(9) << preset.parameters.squeeze_bytes
                      << std::setw(12) << metrics.digest_collisions
                      << std::setw(11) << metrics.state_collisions
                      << std::setw(12) << std::fixed << std::setprecision(3)
                      << metrics.avalanche_mean
                      << std::setw(8) << metrics.avalanche_min
                      << std::setw(8) << metrics.avalanche_max
                      << '\n';
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
