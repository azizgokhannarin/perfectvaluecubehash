#include "pvc/hash.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    try {
        const std::size_t samples =
            argc > 1 ? static_cast<std::size_t>(std::stoull(argv[1])) : 50000U;
        const std::size_t length =
            argc > 2 ? static_cast<std::size_t>(std::stoull(argv[2])) : 16U;

        std::array<std::array<std::uint32_t, 256>, pvc::kCubeCells> counts{};
        std::mt19937_64 generator(0x4355424542494153ULL);
        std::uniform_int_distribution<unsigned> byte_distribution(0U, 255U);
        std::vector<std::uint8_t> message(length);

        for (std::size_t sample = 0; sample < samples; ++sample) {
            for (auto& byte : message) {
                byte = static_cast<std::uint8_t>(byte_distribution(generator));
            }
            const auto result = pvc::RotHash1::inspect(message, false);
            const auto& storage = result.final_cube.storage();
            for (std::size_t position = 0; position < storage.size(); ++position) {
                ++counts[position][storage[position]];
            }
        }

        const auto canonical = pvc::Cube::perfect().storage();
        const long double expected = static_cast<long double>(samples) / 256.0L;
        long double chi_sum = 0.0L;
        long double chi_min = std::numeric_limits<long double>::max();
        long double chi_max = 0.0L;
        long double max_ratio = 0.0L;
        std::size_t canonical_or_complement_preferred = 0U;
        std::size_t min_support = 256U;

        for (std::size_t position = 0; position < counts.size(); ++position) {
            long double chi = 0.0L;
            std::uint32_t maximum = 0U;
            unsigned maximum_value = 0U;
            std::size_t support = 0U;
            for (unsigned value = 0; value < 256U; ++value) {
                const auto count = counts[position][value];
                const long double difference = static_cast<long double>(count) - expected;
                chi += difference * difference / expected;
                if (count != 0U) {
                    ++support;
                }
                if (count > maximum) {
                    maximum = count;
                    maximum_value = value;
                }
            }
            chi_sum += chi;
            chi_min = std::min(chi_min, chi);
            chi_max = std::max(chi_max, chi);
            min_support = std::min(min_support, support);
            max_ratio = std::max(max_ratio, static_cast<long double>(maximum) / expected);
            const auto original = static_cast<unsigned>(canonical[position]);
            if (maximum_value == original || maximum_value == 255U - original) {
                ++canonical_or_complement_preferred;
            }
        }

        std::cout << "PVC-RotHash-1 final-state bias probe\n"
                  << "samples                         : " << samples << '\n'
                  << "message bytes                   : " << length << '\n'
                  << std::fixed << std::setprecision(4)
                  << "per-cell chi-square             : mean="
                  << static_cast<double>(chi_sum / static_cast<long double>(pvc::kCubeCells))
                  << " min=" << static_cast<double>(chi_min)
                  << " max=" << static_cast<double>(chi_max) << '\n'
                  << "minimum per-cell support        : " << min_support << " of 256\n"
                  << "largest value/expected          : " << static_cast<double>(max_ratio) << '\n'
                  << "canonical/complement preferences: "
                  << canonical_or_complement_preferred << " of 512\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
