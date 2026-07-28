#include "pvc/research.hpp"

#include "engine.hpp"

#include <limits>
#include <stdexcept>

namespace pvc {

std::vector<NamedHashParameters> reduced_round_presets() {
    return {
        {"R0-minimal", HashParameters{
            .moves_per_symbol = 1,
            .diagonal_closure_symbols = 0,
            .orbit_closure_symbols = 0,
            .squeeze_bytes = 4,
            .squeeze_symbols_per_byte = 1,
            .enable_foldback = false,
        }},
        {"R1-foldback", HashParameters{
            .moves_per_symbol = 2,
            .diagonal_closure_symbols = 8,
            .orbit_closure_symbols = 0,
            .squeeze_bytes = 8,
            .squeeze_symbols_per_byte = 1,
            .enable_foldback = true,
        }},
        {"R2-small", HashParameters{
            .moves_per_symbol = 3,
            .diagonal_closure_symbols = 16,
            .orbit_closure_symbols = 16,
            .squeeze_bytes = 8,
            .squeeze_symbols_per_byte = 2,
            .enable_foldback = true,
        }},
        {"R3-medium", HashParameters{
            .moves_per_symbol = 4,
            .diagonal_closure_symbols = 32,
            .orbit_closure_symbols = 32,
            .squeeze_bytes = 16,
            .squeeze_symbols_per_byte = 2,
            .enable_foldback = true,
        }},
        {"R4-near", HashParameters{
            .moves_per_symbol = 5,
            .diagonal_closure_symbols = 64,
            .orbit_closure_symbols = 64,
            .squeeze_bytes = 16,
            .squeeze_symbols_per_byte = 4,
            .enable_foldback = true,
        }},
        {"R5-canonical", canonical_hash_parameters()},
    };
}

const char* research_phase_name(ResearchPhase phase) {
    switch (phase) {
    case ResearchPhase::Initial: return "initial";
    case ResearchPhase::AfterForward: return "after-forward";
    case ResearchPhase::AfterFoldback: return "after-foldback";
    case ResearchPhase::AfterDiagonalClosure: return "after-diagonal-closure";
    case ResearchPhase::AfterOrbitClosure: return "after-orbit-closure";
    case ResearchPhase::AfterSqueezeByte: return "after-squeeze-byte";
    case ResearchPhase::Final: return "final";
    }
    return "unknown";
}

void validate_hash_parameters(const HashParameters& parameters) {
    if (parameters.moves_per_symbol == 0U || parameters.moves_per_symbol > 255U) {
        throw std::invalid_argument("moves_per_symbol must be in [1,255]");
    }
    if (parameters.diagonal_closure_symbols > 4096U) {
        throw std::invalid_argument("diagonal_closure_symbols exceeds research limit 4096");
    }
    if (parameters.orbit_closure_symbols > 4096U) {
        throw std::invalid_argument("orbit_closure_symbols exceeds research limit 4096");
    }
    if (parameters.squeeze_bytes == 0U || parameters.squeeze_bytes > kDigestBytes) {
        throw std::invalid_argument("squeeze_bytes must be in [1,32]");
    }
    if (parameters.squeeze_symbols_per_byte == 0U
        || parameters.squeeze_symbols_per_byte > 64U) {
        throw std::invalid_argument("squeeze_symbols_per_byte must be in [1,64]");
    }
}

InternalStateSnapshot initial_internal_state() {
    return detail::snapshot(detail::WorkingState{});
}

InternalStateSnapshot absorb_symbol_for_research(
    const InternalStateSnapshot& state,
    std::uint8_t symbol,
    const HashParameters& parameters,
    std::vector<Move>* trace) {
    validate_hash_parameters(parameters);
    auto working = detail::working_state_from(state, trace);
    detail::absorb_symbol(working, symbol, parameters);
    return detail::snapshot(working);
}

InternalStateSnapshot forward_state_for_research(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters) {
    validate_hash_parameters(parameters);
    detail::WorkingState state;
    for (const auto byte : bytes) {
        detail::absorb_symbol(state, byte, parameters);
    }
    return detail::snapshot(state);
}

ResearchHashResult inspect_with_parameters(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters,
    bool keep_trace,
    bool keep_checkpoints) {
    return detail::compute(bytes, parameters, keep_trace, keep_checkpoints);
}

ResearchHashResult inspect_with_parameters(
    std::string_view text,
    const HashParameters& parameters,
    bool keep_trace,
    bool keep_checkpoints) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    return inspect_with_parameters(
        std::span<const std::uint8_t>{begin, text.size()},
        parameters,
        keep_trace,
        keep_checkpoints);
}

std::vector<std::uint8_t> hash_with_parameters(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters) {
    return detail::compute(bytes, parameters, false, false).digest;
}

} // namespace pvc
