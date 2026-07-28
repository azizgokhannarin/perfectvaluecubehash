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
        << "  " << program << " --file <path>    [--dump-cube] [--trace]\n";
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

        const auto result = pvc::RotHash0::inspect(message, show_trace);

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
