#pragma once

#include "alias_catalog.hpp"
#include "pvc/research.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace pvc::tool {

struct BridgedAliasStep {
    std::optional<std::uint8_t> bridge;
    SymbolAlias alias{};
};

struct BridgedAliasPath {
    ForwardCollisionPair seed{};
    std::vector<BridgedAliasStep> steps;
};

inline std::optional<std::vector<BridgedAliasStep>> find_bridged_tail(
    const InternalStateSnapshot& state,
    const HashParameters& parameters,
    std::size_t remaining_levels) {
    if (remaining_levels == 0U) {
        return std::vector<BridgedAliasStep>{};
    }

    const auto direct_aliases = find_symbol_aliases(state, parameters);
    for (const auto& alias : direct_aliases) {
        auto tail = find_bridged_tail(
            alias.common_state, parameters, remaining_levels - 1U);
        if (tail.has_value()) {
            tail->insert(tail->begin(), BridgedAliasStep{
                .bridge = std::nullopt,
                .alias = alias,
            });
            return tail;
        }
    }

    for (std::uint16_t bridge = 0U; bridge < 256U; ++bridge) {
        const auto bridged = absorb_symbol_for_research(
            state, static_cast<std::uint8_t>(bridge), parameters);
        const auto aliases = find_symbol_aliases(bridged, parameters);
        for (const auto& alias : aliases) {
            auto tail = find_bridged_tail(
                alias.common_state, parameters, remaining_levels - 1U);
            if (tail.has_value()) {
                tail->insert(tail->begin(), BridgedAliasStep{
                    .bridge = static_cast<std::uint8_t>(bridge),
                    .alias = alias,
                });
                return tail;
            }
        }
    }
    return std::nullopt;
}

inline std::optional<BridgedAliasPath> find_bridged_path(
    const std::vector<ForwardCollisionPair>& seeds,
    const HashParameters& parameters,
    std::size_t levels) {
    if (levels < 1U) {
        return std::nullopt;
    }
    for (const auto& seed : seeds) {
        auto tail = find_bridged_tail(
            seed.common_state, parameters, levels - 1U);
        if (tail.has_value()) {
            return BridgedAliasPath{.seed = seed, .steps = std::move(*tail)};
        }
    }
    return std::nullopt;
}

inline InternalStateSnapshot path_common_state(
    const BridgedAliasPath& path,
    const HashParameters& parameters,
    std::size_t levels) {
    auto state = path.seed.common_state;
    const auto used = std::min(levels > 0U ? levels - 1U : 0U,
                               path.steps.size());
    for (std::size_t i = 0U; i < used; ++i) {
        if (path.steps[i].bridge.has_value()) {
            state = absorb_symbol_for_research(
                state, *path.steps[i].bridge, parameters);
        }
        state = path.steps[i].alias.common_state;
    }
    return state;
}

} // namespace pvc::tool
