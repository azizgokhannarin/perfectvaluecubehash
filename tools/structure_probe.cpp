#include "pvc/hash.hpp"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
    const std::string text = argc > 1 ? argv[1] : "Perfect Value Cube";
    const std::vector<std::uint8_t> message(text.begin(), text.end());
    const auto result = pvc::RotHash0::inspect(message, true);

    bool axes_alternate = true;
    bool intersections_chain = true;

    for (std::size_t i = 1; i < result.trace.size(); ++i) {
        const auto& previous = result.trace[i - 1U];
        const auto& current = result.trace[i];

        if (previous.axis == current.axis) {
            axes_alternate = false;
        }
        if (!pvc::line_contains(previous.axis,
                                previous.intersection_before,
                                current.intersection_before)) {
            intersections_chain = false;
        }
    }

    const auto histogram_ok = result.final_cube.has_double_byte_histogram();
    const auto still_balanced = result.final_cube.is_balanced();

    std::cout << "PVC-RotHash-0 structure probe\n"
              << "moves                         : " << result.trace.size() << '\n'
              << "consecutive axes differ       : " << (axes_alternate ? "yes" : "no") << '\n'
              << "each line meets previous line : " << (intersections_chain ? "yes" : "no") << '\n'
              << "0..255 each still occur twice : " << (histogram_ok ? "yes" : "no") << '\n'
              << "all 192 sums still 1020       : " << (still_balanced ? "yes" : "no") << '\n';

    return (axes_alternate && intersections_chain && histogram_ok) ? 0 : 1;
}
