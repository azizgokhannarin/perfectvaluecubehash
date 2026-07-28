#include "pvc/hash.hpp"
#include "pvc/hex.hpp"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    const std::string text = argc > 1 ? argv[1] : "Perfect Value Cube";
    std::vector<std::uint8_t> message(text.begin(), text.end());

    if (message.empty()) {
        std::cerr << "The avalanche probe requires a non-empty message.\n";
        return 2;
    }

    const auto baseline = pvc::RotHash0::hash(message);
    std::vector<std::size_t> bit_distances;
    std::vector<std::size_t> byte_distances;
    bit_distances.reserve(message.size() * 8U);
    byte_distances.reserve(message.size() * 8U);

    for (std::size_t byte_index = 0; byte_index < message.size(); ++byte_index) {
        for (unsigned bit = 0; bit < 8U; ++bit) {
            auto changed = message;
            changed[byte_index] ^= static_cast<std::uint8_t>(1U << bit);
            const auto digest = pvc::RotHash0::hash(changed);
            bit_distances.push_back(pvc::digest_hamming_distance(baseline, digest));
            byte_distances.push_back(pvc::digest_byte_distance(baseline, digest));
        }
    }

    const auto [min_bit, max_bit] =
        std::minmax_element(bit_distances.begin(), bit_distances.end());
    const auto [min_byte, max_byte] =
        std::minmax_element(byte_distances.begin(), byte_distances.end());

    const double mean_bit =
        static_cast<double>(std::accumulate(bit_distances.begin(), bit_distances.end(), std::size_t{0}))
        / static_cast<double>(bit_distances.size());
    const double mean_byte =
        static_cast<double>(std::accumulate(byte_distances.begin(), byte_distances.end(), std::size_t{0}))
        / static_cast<double>(byte_distances.size());

    std::cout << "PVC-RotHash-0 avalanche probe\n"
              << "message bytes : " << message.size() << '\n'
              << "baseline      : " << pvc::to_hex(baseline) << '\n'
              << "trials        : " << bit_distances.size() << '\n'
              << std::fixed << std::setprecision(2)
              << "bit distance  : mean=" << mean_bit
              << " min=" << *min_bit
              << " max=" << *max_bit
              << " of 256\n"
              << "byte distance : mean=" << mean_byte
              << " min=" << *min_byte
              << " max=" << *max_byte
              << " of 32\n";

    return 0;
}
