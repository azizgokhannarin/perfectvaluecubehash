#include "pvc/hash.hpp"

#include <algorithm>
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
        const std::size_t trials =
            argc > 1 ? static_cast<std::size_t>(std::stoull(argv[1])) : 32000U;
        const std::size_t length =
            argc > 2 ? static_cast<std::size_t>(std::stoull(argv[2])) : 16U;
        if (length == 0U) {
            std::cerr << "message length must be positive\n";
            return 2;
        }

        std::mt19937_64 generator(0x4156414c414e4348ULL);
        std::uniform_int_distribution<unsigned> byte_distribution(0U, 255U);
        std::uniform_int_distribution<std::size_t> bit_distribution(0U, length * 8U - 1U);
        std::vector<std::uint8_t> message(length);

        long double sum = 0.0L;
        long double square_sum = 0.0L;
        std::size_t minimum = std::numeric_limits<std::size_t>::max();
        std::size_t maximum = 0U;

        for (std::size_t trial = 0; trial < trials; ++trial) {
            for (auto& byte : message) {
                byte = static_cast<std::uint8_t>(byte_distribution(generator));
            }
            const auto baseline = pvc::RotHash1::hash(message);
            const auto changed_bit = bit_distribution(generator);
            message[changed_bit >> 3U] ^=
                static_cast<std::uint8_t>(1U << static_cast<unsigned>(changed_bit & 7U));
            const auto changed = pvc::RotHash1::hash(message);
            const auto distance = pvc::digest_hamming_distance(baseline, changed);
            sum += static_cast<long double>(distance);
            square_sum += static_cast<long double>(distance * distance);
            minimum = std::min(minimum, distance);
            maximum = std::max(maximum, distance);
        }

        const auto count = static_cast<long double>(trials);
        const auto mean = sum / count;
        const auto variance = square_sum / count - mean * mean;
        const auto deviation = std::sqrt(std::max(variance, 0.0L));

        std::cout << "PVC-RotHash-1 avalanche sweep\n"
                  << "trials       : " << trials << '\n'
                  << "message bytes: " << length << '\n'
                  << std::fixed << std::setprecision(4)
                  << "bit distance : mean=" << static_cast<double>(mean)
                  << " sd=" << static_cast<double>(deviation)
                  << " min=" << minimum
                  << " max=" << maximum
                  << " of 256\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
