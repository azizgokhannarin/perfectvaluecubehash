#pragma once

#include "pvc/hash.hpp"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace pvc {

// Runtime parameters exist only for cryptanalysis. The public RotHash1 API
// always uses canonical_hash_parameters() and therefore remains stable.
struct HashParameters {
    std::size_t moves_per_symbol = kMovesPerSymbol;
    std::size_t diagonal_closure_symbols = kClosureSymbols;
    std::size_t orbit_closure_symbols = kOrbitSymbols;
    std::size_t squeeze_bytes = kDigestBytes;
    std::size_t squeeze_symbols_per_byte = kSqueezeSymbolsPerByte;
    bool enable_foldback = true;

    friend constexpr bool operator==(const HashParameters&, const HashParameters&) = default;
};

struct InternalStateSnapshot {
    Cube cube{};
    Coord cursor{};
    Axis previous_axis = Axis::X;
    std::uint64_t symbol_index = 0;

    friend bool operator==(const InternalStateSnapshot&, const InternalStateSnapshot&) = default;
};

enum class ResearchPhase : std::uint8_t {
    Initial,
    AfterForward,
    AfterFoldback,
    AfterDiagonalClosure,
    AfterOrbitClosure,
    AfterSqueezeByte,
    Final,
};

struct StateCheckpoint {
    ResearchPhase phase = ResearchPhase::Initial;
    std::size_t index = 0;
    InternalStateSnapshot state{};
};

struct ResearchHashResult {
    std::vector<std::uint8_t> digest{};
    InternalStateSnapshot final_state{};
    std::vector<StateCheckpoint> checkpoints{};
    std::vector<Move> trace{};
    std::uint64_t input_size = 0;
};

struct NamedHashParameters {
    std::string_view name{};
    HashParameters parameters{};
};

[[nodiscard]] constexpr HashParameters canonical_hash_parameters() {
    return HashParameters{};
}

[[nodiscard]] std::vector<NamedHashParameters> reduced_round_presets();
[[nodiscard]] const char* research_phase_name(ResearchPhase phase);

// Throws std::invalid_argument for unsupported research configurations.
void validate_hash_parameters(const HashParameters& parameters);

[[nodiscard]] InternalStateSnapshot initial_internal_state();

// Applies exactly one absorbed symbol to an explicit operational state.
// This is the primitive used by transition-collision and predecessor tools.
[[nodiscard]] InternalStateSnapshot absorb_symbol_for_research(
    const InternalStateSnapshot& state,
    std::uint8_t symbol,
    const HashParameters& parameters,
    std::vector<Move>* trace = nullptr);

// Forward-only state after absorbing the supplied bytes. No foldback,
// finalization closure, orbit closure, or squeeze is performed.
[[nodiscard]] InternalStateSnapshot forward_state_for_research(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters);

// State after forward absorption and the optional reverse foldback. No closure
// or squeeze is performed. Equal snapshots for equal-length messages imply
// identical remaining finalization and therefore a full digest collision.
[[nodiscard]] InternalStateSnapshot foldback_state_for_research(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters);

[[nodiscard]] ResearchHashResult inspect_with_parameters(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters,
    bool keep_trace = false,
    bool keep_checkpoints = true);

[[nodiscard]] ResearchHashResult inspect_with_parameters(
    std::string_view text,
    const HashParameters& parameters,
    bool keep_trace = false,
    bool keep_checkpoints = true);

[[nodiscard]] std::vector<std::uint8_t> hash_with_parameters(
    std::span<const std::uint8_t> bytes,
    const HashParameters& parameters);

} // namespace pvc
