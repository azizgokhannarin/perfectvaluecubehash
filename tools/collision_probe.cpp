#include "pvc/hash.hpp"
#include "pvc/hex.hpp"

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace {

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

bool probe_one_byte() {
    std::map<pvc::Digest, std::vector<std::uint8_t>> seen;
    for (unsigned value = 0; value < 256U; ++value) {
        const std::vector<std::uint8_t> message{
            static_cast<std::uint8_t>(value)
        };
        const auto digest = pvc::RotHash1::hash(message);
        const auto [it, inserted] = seen.emplace(digest, message);
        if (!inserted) {
            std::cout << "collision found\n"
                      << "a=" << describe(it->second) << '\n'
                      << "b=" << describe(message) << '\n'
                      << "h=" << pvc::to_hex(digest) << '\n';
            return false;
        }
    }
    std::cout << "No collision among all 256 one-byte inputs.\n";
    return true;
}

bool probe_two_bytes() {
    std::map<pvc::Digest, std::array<std::uint8_t, 2>> seen;
    for (unsigned first = 0; first < 256U; ++first) {
        for (unsigned second = 0; second < 256U; ++second) {
            const std::array<std::uint8_t, 2> message{
                static_cast<std::uint8_t>(first),
                static_cast<std::uint8_t>(second)
            };
            const auto digest = pvc::RotHash1::hash(message);
            const auto [it, inserted] = seen.emplace(digest, message);
            if (!inserted) {
                const std::vector<std::uint8_t> a{it->second[0], it->second[1]};
                const std::vector<std::uint8_t> b{message[0], message[1]};
                std::cout << "collision found\n"
                          << "a=" << describe(a) << '\n'
                          << "b=" << describe(b) << '\n'
                          << "h=" << pvc::to_hex(digest) << '\n';
                return false;
            }
        }
    }
    std::cout << "No collision among all 65,536 two-byte inputs.\n";
    return true;
}

} // namespace

int main(int argc, char** argv) {
    const std::string mode = argc > 1 ? argv[1] : "1";

    if (mode == "1") {
        return probe_one_byte() ? 0 : 1;
    }
    if (mode == "2") {
        return probe_two_bytes() ? 0 : 1;
    }

    std::cerr << "Usage: pvc-collision-probe [1|2]\n";
    return 2;
}
