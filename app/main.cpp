#include "pvc/hash.hpp"
#include "pvc/hex.hpp"

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <vector>

namespace {

void print_usage(const char* program) {
    std::cerr
        << "Usage:\n"
        << "  " << program << " --text <message> [--dump-cube] [--trace]\n"
        << "  " << program << " --file <path>    [--dump-cube] [--trace]\n"
        << "  " << program << " --hex <bytes>    [--dump-cube] [--trace]\n";
}

std::vector<std::uint8_t> parse_hex(const std::string& text) {
    if ((text.size() & 1U) != 0U) {
        throw std::invalid_argument("Hex input must contain an even number of characters");
    }
    const auto nibble = [](char value) -> std::uint8_t {
        if (value >= '0' && value <= '9') return static_cast<std::uint8_t>(value - '0');
        if (value >= 'a' && value <= 'f') return static_cast<std::uint8_t>(value - 'a' + 10);
        if (value >= 'A' && value <= 'F') return static_cast<std::uint8_t>(value - 'A' + 10);
        throw std::invalid_argument("Hex input contains a non-hexadecimal character");
    };

    std::vector<std::uint8_t> result(text.size() / 2U);
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<std::uint8_t>((nibble(text[i * 2U]) << 4U)
                                            | nibble(text[i * 2U + 1U]));
    }
    return result;
}

std::vector<std::uint8_t> read_file(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        throw std::runtime_error("Unable to open file: " + path);
    }

    return std::vector<std::uint8_t>(
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>());
}

} // namespace

int main(int argc, char** argv) {
    try {
        std::vector<std::uint8_t> message;
        bool dump_cube = false;
        bool show_trace = false;
        bool have_input = false;

        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];
            if (arg == "--text" && i + 1 < argc) {
                const std::string text = argv[++i];
                message.assign(text.begin(), text.end());
                have_input = true;
            } else if (arg == "--file" && i + 1 < argc) {
                message = read_file(argv[++i]);
                have_input = true;
            } else if (arg == "--hex" && i + 1 < argc) {
                message = parse_hex(argv[++i]);
                have_input = true;
            } else if (arg == "--dump-cube") {
                dump_cube = true;
            } else if (arg == "--trace") {
                show_trace = true;
            } else if (arg == "--help" || arg == "-h") {
                print_usage(argv[0]);
                return 0;
            } else {
                print_usage(argv[0]);
                return 2;
            }
        }

        if (!have_input) {
            print_usage(argv[0]);
            return 2;
        }

        const auto result = pvc::RotHash1::inspect(message, show_trace);

        std::cout << pvc::to_hex(result.digest) << '\n';

        if (show_trace) {
            std::cerr << "moves=" << result.trace.size() << '\n';
            for (const auto& move : result.trace) {
                std::cerr
                    << "symbol=" << move.symbol_index
                    << " phase=" << static_cast<unsigned>(move.phase)
                    << " axis=" << pvc::axis_name(move.axis)
                    << " amount=" << static_cast<unsigned>(move.amount)
                    << " at=("
                    << static_cast<unsigned>(move.intersection_before.x) << ','
                    << static_cast<unsigned>(move.intersection_before.y) << ','
                    << static_cast<unsigned>(move.intersection_before.z) << ")"
                    << " -> ("
                    << static_cast<unsigned>(move.intersection_after.x) << ','
                    << static_cast<unsigned>(move.intersection_after.y) << ','
                    << static_cast<unsigned>(move.intersection_after.z) << ")\n";
            }
        }

        if (dump_cube) {
            result.final_cube.print_layers(std::cout);
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
