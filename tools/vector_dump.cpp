#include "pvc/hex.hpp"
#include "pvc/research.hpp"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::uint8_t nibble(char value) {
    if (value >= '0' && value <= '9') return static_cast<std::uint8_t>(value - '0');
    if (value >= 'a' && value <= 'f') return static_cast<std::uint8_t>(value - 'a' + 10);
    if (value >= 'A' && value <= 'F') return static_cast<std::uint8_t>(value - 'A' + 10);
    throw std::invalid_argument("non-hexadecimal input");
}

std::vector<std::uint8_t> parse_hex(const std::string& text) {
    if ((text.size() & 1U) != 0U) {
        throw std::invalid_argument("hex input must have even length");
    }
    std::vector<std::uint8_t> result(text.size() / 2U);
    for (std::size_t i = 0; i < result.size(); ++i) {
        result[i] = static_cast<std::uint8_t>((nibble(text[i * 2U]) << 4U)
                                            | nibble(text[i * 2U + 1U]));
    }
    return result;
}

template <typename Range>
std::string bytes_hex(const Range& bytes) {
    static constexpr char alphabet[] = "0123456789abcdef";
    std::string result;
    result.reserve(bytes.size() * 2U);
    for (const auto value : bytes) {
        const auto byte = static_cast<std::uint8_t>(value);
        result.push_back(alphabet[(byte >> 4U) & 0x0fU]);
        result.push_back(alphabet[byte & 0x0fU]);
    }
    return result;
}

std::string axis_json(pvc::Axis axis) {
    switch (axis) {
    case pvc::Axis::X: return "X";
    case pvc::Axis::Y: return "Y";
    case pvc::Axis::Z: return "Z";
    }
    return "?";
}

void print_state(const pvc::InternalStateSnapshot& state) {
    std::cout << "{\"cube_hex\":\"" << bytes_hex(state.cube.storage())
              << "\",\"cursor\":["
              << static_cast<unsigned>(state.cursor.x) << ','
              << static_cast<unsigned>(state.cursor.y) << ','
              << static_cast<unsigned>(state.cursor.z) << "],\"previous_axis\":\""
              << axis_json(state.previous_axis) << "\",\"symbol_index\":"
              << state.symbol_index << '}';
}

const pvc::InternalStateSnapshot& phase_state(const pvc::ResearchHashResult& result,
                                              pvc::ResearchPhase phase) {
    if (phase == pvc::ResearchPhase::Final) {
        return result.final_state;
    }
    for (const auto& checkpoint : result.checkpoints) {
        if (checkpoint.phase == phase) {
            return checkpoint.state;
        }
    }
    throw std::logic_error("phase checkpoint missing");
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc != 3 || std::string{argv[1]} != "--hex") {
            std::cerr << "Usage: " << argv[0] << " --hex <message-hex>\n";
            return 2;
        }
        const auto message = parse_hex(argv[2]);
        const auto result = pvc::inspect_with_parameters(
            message, pvc::canonical_hash_parameters(), false, true);

        std::cout << "{\n"
                  << "  \"input_hex\": \"" << bytes_hex(message) << "\",\n"
                  << "  \"input_length\": " << message.size() << ",\n"
                  << "  \"digest_hex\": \"" << bytes_hex(result.digest) << "\",\n"
                  << "  \"after_forward\": ";
        print_state(phase_state(result, pvc::ResearchPhase::AfterForward));
        std::cout << ",\n  \"after_foldback\": ";
        print_state(phase_state(result, pvc::ResearchPhase::AfterFoldback));
        std::cout << ",\n  \"after_diagonal_closure\": ";
        print_state(phase_state(result, pvc::ResearchPhase::AfterDiagonalClosure));
        std::cout << ",\n  \"after_orbit_closure\": ";
        print_state(phase_state(result, pvc::ResearchPhase::AfterOrbitClosure));
        std::cout << ",\n  \"final\": ";
        print_state(result.final_state);
        std::cout << "\n}\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
