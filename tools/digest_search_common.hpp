#pragma once

#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace pvc::tool {

inline std::vector<std::uint8_t> parse_hex_message(std::string_view text) {
    if ((text.size() & 1U) != 0U) {
        throw std::invalid_argument("hex message must contain an even number of digits");
    }
    auto nibble = [](char c) -> std::uint8_t {
        if (c >= '0' && c <= '9') return static_cast<std::uint8_t>(c - '0');
        if (c >= 'a' && c <= 'f') return static_cast<std::uint8_t>(10 + c - 'a');
        if (c >= 'A' && c <= 'F') return static_cast<std::uint8_t>(10 + c - 'A');
        throw std::invalid_argument("invalid hexadecimal digit");
    };
    std::vector<std::uint8_t> result;
    result.reserve(text.size() / 2U);
    for (std::size_t i = 0U; i < text.size(); i += 2U) {
        result.push_back(static_cast<std::uint8_t>(
            static_cast<unsigned>(nibble(text[i])) * 16U
            + static_cast<unsigned>(nibble(text[i + 1U]))));
    }
    return result;
}

inline long double binomial_probability_256(std::size_t distance) {
    if (distance > 256U) {
        return 0.0L;
    }
    long double term = std::ldexp(1.0L, -256);
    long double total = term;
    for (std::size_t k = 0U; k < distance; ++k) {
        term *= static_cast<long double>(256U - k)
              / static_cast<long double>(k + 1U);
        total += term;
    }
    return total;
}

inline std::size_t generic_minimum_distance(std::uint64_t comparisons) {
    if (comparisons == 0U) {
        return 256U;
    }
    for (std::size_t distance = 0U; distance <= 256U; ++distance) {
        const long double expected = static_cast<long double>(comparisons)
                                   * binomial_probability_256(distance);
        if (expected >= 1.0L) {
            return distance;
        }
    }
    return 256U;
}

inline const InternalStateSnapshot& phase_state(
    const ResearchHashResult& result,
    ResearchPhase phase,
    std::size_t index = std::numeric_limits<std::size_t>::max()) {
    for (const auto& checkpoint : result.checkpoints) {
        if (checkpoint.phase == phase
            && (index == std::numeric_limits<std::size_t>::max()
                || checkpoint.index == index)) {
            return checkpoint.state;
        }
    }
    throw std::runtime_error("requested phase checkpoint is unavailable");
}

inline void print_phase_distance_profile(
    std::span<const std::uint8_t> left,
    std::span<const std::uint8_t> right,
    const HashParameters& parameters) {
    const auto left_result = inspect_with_parameters(left, parameters, false, true);
    const auto right_result = inspect_with_parameters(right, parameters, false, true);
    const std::array<ResearchPhase, 5> phases{
        ResearchPhase::AfterForward,
        ResearchPhase::AfterFoldback,
        ResearchPhase::AfterDiagonalClosure,
        ResearchPhase::AfterOrbitClosure,
        ResearchPhase::Final,
    };
    for (const auto phase : phases) {
        const auto& left_state = phase_state(left_result, phase);
        const auto& right_state = phase_state(right_result, phase);
        std::cout << "phase=" << research_phase_name(phase)
                  << " state_bits="
                  << operational_state_bit_distance(left_state, right_state)
                  << " state_bytes="
                  << operational_state_byte_distance(left_state, right_state)
                  << '\n';
    }
    const auto squeeze_count = std::min(
        left_result.digest.size(), right_result.digest.size());
    for (std::size_t i = 0U; i < squeeze_count; ++i) {
        const auto& left_state = phase_state(
            left_result, ResearchPhase::AfterSqueezeByte, i);
        const auto& right_state = phase_state(
            right_result, ResearchPhase::AfterSqueezeByte, i);
        std::cout << "phase=after-squeeze-byte index=" << i
                  << " state_bits="
                  << operational_state_bit_distance(left_state, right_state)
                  << " emitted_prefix_bits="
                  << byte_hamming_distance(
                        std::span<const std::uint8_t>{left_result.digest.data(), i + 1U},
                        std::span<const std::uint8_t>{right_result.digest.data(), i + 1U})
                  << '\n';
    }
}

} // namespace pvc::tool
