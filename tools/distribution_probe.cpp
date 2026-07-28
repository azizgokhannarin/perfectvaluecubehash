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

namespace {

struct Accumulator {
    std::array<std::array<std::uint64_t, 256>, pvc::kDigestBytes> byte_counts{};
    std::array<std::uint64_t, pvc::kDigestBytes * 8U> bit_ones{};
    std::uint64_t messages_with_triple{};
    long double equal_pairs_total{};
    long double complement_pairs_total{};
    std::uint64_t samples{};

    void add(const pvc::Digest& digest) {
        std::array<unsigned, 256> local{};
        for (std::size_t position = 0; position < digest.size(); ++position) {
            ++byte_counts[position][digest[position]];
            ++local[digest[position]];
            for (unsigned bit = 0; bit < 8U; ++bit) {
                bit_ones[position * 8U + bit] +=
                    static_cast<std::uint64_t>((digest[position] >> bit) & 1U);
            }
        }

        bool triple = false;
        for (const auto value_count : local) {
            if (value_count >= 3U) {
                triple = true;
            }
            equal_pairs_total +=
                static_cast<long double>(value_count * (value_count - 1U) / 2U);
        }

        for (std::size_t left = 0; left < digest.size(); ++left) {
            for (std::size_t right = left + 1U; right < digest.size(); ++right) {
                if (static_cast<unsigned>(digest[left])
                      + static_cast<unsigned>(digest[right]) == 255U) {
                    complement_pairs_total += 1.0L;
                }
            }
        }

        messages_with_triple += triple ? 1U : 0U;
        ++samples;
    }
};

void print_report(const Accumulator& data, const std::string& source) {
    const long double expected = static_cast<long double>(data.samples) / 256.0L;
    long double chi_sum = 0.0L;
    long double chi_min = std::numeric_limits<long double>::max();
    long double chi_max = 0.0L;
    std::size_t min_support = 256U;
    std::size_t max_support = 0U;
    long double max_ratio = 0.0L;

    for (const auto& position_counts : data.byte_counts) {
        long double chi = 0.0L;
        std::size_t support = 0U;
        std::uint64_t maximum = 0U;
        for (const auto count : position_counts) {
            const long double difference = static_cast<long double>(count) - expected;
            chi += difference * difference / expected;
            if (count != 0U) {
                ++support;
            }
            maximum = std::max(maximum, count);
        }
        chi_sum += chi;
        chi_min = std::min(chi_min, chi);
        chi_max = std::max(chi_max, chi);
        min_support = std::min(min_support, support);
        max_support = std::max(max_support, support);
        max_ratio = std::max(max_ratio, static_cast<long double>(maximum) / expected);
    }

    long double max_bit_z = 0.0L;
    const long double bit_expected = static_cast<long double>(data.samples) / 2.0L;
    const long double bit_sigma = std::sqrt(static_cast<long double>(data.samples) / 4.0L);
    for (const auto ones : data.bit_ones) {
        const auto z = std::fabs((static_cast<long double>(ones) - bit_expected) / bit_sigma);
        max_bit_z = std::max(max_bit_z, z);
    }

    const long double sample_count = static_cast<long double>(data.samples);
    std::cout << "PVC-RotHash-1 distribution probe\n"
              << "source                     : " << source << '\n'
              << "samples                    : " << data.samples << '\n'
              << std::fixed << std::setprecision(4)
              << "per-position chi-square    : mean="
              << static_cast<double>(chi_sum / static_cast<long double>(pvc::kDigestBytes))
              << " min=" << static_cast<double>(chi_min)
              << " max=" << static_cast<double>(chi_max) << '\n'
              << "support                     : min=" << min_support
              << " max=" << max_support << " of 256\n"
              << "largest value/expected     : " << static_cast<double>(max_ratio) << '\n'
              << "maximum output-bit |z|     : " << static_cast<double>(max_bit_z) << '\n'
              << "equal byte pairs/digest    : "
              << static_cast<double>(data.equal_pairs_total / sample_count)
              << " (random expectation 1.9375)\n"
              << "complement pairs/digest    : "
              << static_cast<double>(data.complement_pairs_total / sample_count)
              << " (random expectation 1.9375)\n"
              << "digests with >=3 equal byte: "
              << static_cast<double>(100.0L
                   * static_cast<long double>(data.messages_with_triple) / sample_count)
              << "%\n";
}

Accumulator random_messages(std::size_t samples, std::size_t length) {
    Accumulator data;
    std::mt19937_64 generator(0x5056434841534831ULL);
    std::uniform_int_distribution<unsigned> byte_distribution(0U, 255U);
    std::vector<std::uint8_t> message(length);

    for (std::size_t sample = 0; sample < samples; ++sample) {
        for (auto& byte : message) {
            byte = static_cast<std::uint8_t>(byte_distribution(generator));
        }
        data.add(pvc::RotHash1::hash(message));
    }
    return data;
}

Accumulator exhaustive_two_byte() {
    Accumulator data;
    for (unsigned first = 0; first < 256U; ++first) {
        for (unsigned second = 0; second < 256U; ++second) {
            const std::array<std::uint8_t, 2> message{
                static_cast<std::uint8_t>(first),
                static_cast<std::uint8_t>(second),
            };
            data.add(pvc::RotHash1::hash(message));
        }
    }
    return data;
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc > 1 && std::string{argv[1]} == "--exhaustive-two-byte") {
            const auto data = exhaustive_two_byte();
            print_report(data, "all 65,536 two-byte messages");
            return 0;
        }

        const std::size_t samples =
            argc > 1 ? static_cast<std::size_t>(std::stoull(argv[1])) : 100000U;
        const std::size_t length =
            argc > 2 ? static_cast<std::size_t>(std::stoull(argv[2])) : 16U;
        const auto data = random_messages(samples, length);
        print_report(data, std::to_string(samples) + " deterministic test messages of length "
                           + std::to_string(length));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 2;
    }
}
