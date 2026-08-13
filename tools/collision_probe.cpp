#include "pvc/hash.hpp"
#include "pvc/hex.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace {

using HashFn = pvc::Digest (*)(std::span<const std::uint8_t>);

std::string describe(const std::vector<std::uint8_t>& message) {
    std::string result;
    for (std::size_t i = 0; i < message.size(); ++i) {
        if (i != 0U) {
            result += ' ';
        }
        constexpr char hex[] = "0123456789abcdef";
        result += hex[(message[i] >> 4U) & 0x0FU];
        result += hex[message[i] & 0x0FU];
    }
    return result;
}

pvc::Digest hash_rothash1(std::span<const std::uint8_t> message) {
    return pvc::RotHash1::hash(message);
}

pvc::Digest hash_rothash2(std::span<const std::uint8_t> message) {
    return pvc::RotHash2::hash(message);
}

bool probe_one_byte(HashFn hash, std::string_view algorithm) {
    std::map<pvc::Digest, std::vector<std::uint8_t>> seen;
    for (unsigned value = 0; value < 256U; ++value) {
        const std::vector<std::uint8_t> message{
            static_cast<std::uint8_t>(value)
        };
        const auto digest = hash(message);
        const auto [it, inserted] = seen.emplace(digest, message);
        if (!inserted) {
            std::cout << "collision found (" << algorithm << ")\n"
                      << "a=" << describe(it->second) << '\n'
                      << "b=" << describe(message) << '\n'
                      << "h=" << pvc::to_hex(digest) << '\n';
            return false;
        }
    }
    std::cout << "No collision among all 256 one-byte inputs (" << algorithm << ").\n";
    return true;
}

bool probe_two_bytes(HashFn hash, std::string_view algorithm) {
    std::map<pvc::Digest, std::array<std::uint8_t, 2>> seen;
    for (unsigned first = 0; first < 256U; ++first) {
        for (unsigned second = 0; second < 256U; ++second) {
            const std::array<std::uint8_t, 2> message{
                static_cast<std::uint8_t>(first),
                static_cast<std::uint8_t>(second)
            };
            const auto digest = hash(message);
            const auto [it, inserted] = seen.emplace(digest, message);
            if (!inserted) {
                const std::vector<std::uint8_t> a{it->second[0], it->second[1]};
                const std::vector<std::uint8_t> b{message[0], message[1]};
                std::cout << "collision found (" << algorithm << ")\n"
                          << "a=" << describe(a) << '\n'
                          << "b=" << describe(b) << '\n'
                          << "h=" << pvc::to_hex(digest) << '\n';
                return false;
            }
        }
    }
    std::cout << "No collision among all 65,536 two-byte inputs (" << algorithm << ").\n";
    return true;
}

// Known RotHash-1 two-byte forward aliases: digests must differ under the
// selected algorithm (for RotHash-2 this is a regression smoke check).
bool probe_known_r1_forward_pair(HashFn hash, std::string_view algorithm) {
    const std::array<std::uint8_t, 2> left{{0x17U, 0x6fU}};
    const std::array<std::uint8_t, 2> right{{0x17U, 0x99U}};
    const auto left_digest = hash(left);
    const auto right_digest = hash(right);
    if (left_digest == right_digest) {
        std::cout << "known r1 forward pair collides under " << algorithm << "\n"
                  << "h=" << pvc::to_hex(left_digest) << '\n';
        return false;
    }
    std::cout << "Known RotHash-1 forward pair digests differ under " << algorithm << ".\n";
    return true;
}

} // namespace

int main(int argc, char** argv) {
    bool use_rothash2 = false;
    std::string mode = "1";

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--rothash2") {
            use_rothash2 = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cerr
                << "Usage: pvc-collision-probe [--rothash2] [1|2|r1-pair]\n"
                << "  1       exhaustive one-byte digests (default)\n"
                << "  2       exhaustive two-byte digests\n"
                << "  r1-pair known RotHash-1 forward pair must not collide\n"
                << "Default algorithm: PVC-RotHash-1. --rothash2 selects RotHash-2.\n";
            return 0;
        } else if (arg == "1" || arg == "2" || arg == "r1-pair") {
            mode = arg;
        } else {
            std::cerr << "Usage: pvc-collision-probe [--rothash2] [1|2|r1-pair]\n";
            return 2;
        }
    }

    const HashFn hash = use_rothash2 ? hash_rothash2 : hash_rothash1;
    const std::string_view algorithm = use_rothash2 ? "RotHash-2" : "RotHash-1";

    if (mode == "1") {
        return probe_one_byte(hash, algorithm) ? 0 : 1;
    }
    if (mode == "2") {
        return probe_two_bytes(hash, algorithm) ? 0 : 1;
    }
    if (mode == "r1-pair") {
        return probe_known_r1_forward_pair(hash, algorithm) ? 0 : 1;
    }

    std::cerr << "Usage: pvc-collision-probe [--rothash2] [1|2|r1-pair]\n";
    return 2;
}
