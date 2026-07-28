#pragma once

#include "pvc/research.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace pvc::detail {

struct WorkingState {
    Cube cube = Cube::perfect();
    Coord cursor{0, 0, 0};
    Axis previous_axis = Axis::X;
    std::uint64_t symbol_index = 0;
    std::vector<Move>* trace = nullptr;
};

[[nodiscard]] InternalStateSnapshot snapshot(const WorkingState& state);
[[nodiscard]] WorkingState working_state_from(const InternalStateSnapshot& state,
                                              std::vector<Move>* trace = nullptr);

void absorb_symbol(WorkingState& state,
                   std::uint8_t symbol,
                   const HashParameters& parameters);

void absorb_forward(WorkingState& state,
                    std::span<const std::uint8_t> bytes,
                    const HashParameters& parameters);

void absorb_foldback(WorkingState& state,
                     std::span<const std::uint8_t> bytes,
                     const HashParameters& parameters);

[[nodiscard]] ResearchHashResult compute(std::span<const std::uint8_t> bytes,
                                         const HashParameters& parameters,
                                         bool keep_trace,
                                         bool keep_checkpoints);

} // namespace pvc::detail
