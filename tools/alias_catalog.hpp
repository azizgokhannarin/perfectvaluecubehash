#pragma once

#include "pvc/research.hpp"
#include "research_tool_common.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace pvc::tool {

struct SymbolAlias {
    std::uint8_t left = 0;
    std::uint8_t right = 0;
    InternalStateSnapshot common_state{};
};

struct ForwardCollisionPair {
    std::array<std::uint8_t, 3> left{};
    std::array<std::uint8_t, 3> right{};
    InternalStateSnapshot common_state{};
    std::size_t differing_index = 0;
    bool inherited = false;
};

inline std::vector<SymbolAlias> find_symbol_aliases(
    const InternalStateSnapshot& start,
    const HashParameters& parameters) {
    std::array<InternalStateSnapshot, 256> states{};
    std::unordered_map<std::uint64_t, std::vector<std::uint16_t>> buckets;
    buckets.reserve(512U);
    std::vector<SymbolAlias> aliases;

    for (std::uint16_t symbol = 0; symbol < 256U; ++symbol) {
        auto& state = states[symbol];
        state = absorb_symbol_for_research(
            start, static_cast<std::uint8_t>(symbol), parameters);
        const auto fingerprint = fast_state_fingerprint64(state);
        auto& candidates = buckets[fingerprint];
        for (const auto prior : candidates) {
            if (states[prior] == state) {
                aliases.push_back(SymbolAlias{
                    .left = static_cast<std::uint8_t>(prior),
                    .right = static_cast<std::uint8_t>(symbol),
                    .common_state = state,
                });
            }
        }
        candidates.push_back(symbol);
    }
    return aliases;
}

inline std::array<std::uint8_t, 2> prefix_bytes(std::uint32_t prefix) {
    return {
        static_cast<std::uint8_t>((prefix >> 8U) & 0xFFU),
        static_cast<std::uint8_t>(prefix & 0xFFU),
    };
}

inline std::vector<ForwardCollisionPair> enumerate_local_three_byte_aliases(
    const HashParameters& parameters,
    std::size_t prefix_count = 65536U,
    std::size_t thread_count = 0U) {
    prefix_count = std::min<std::size_t>(prefix_count, 65536U);
    if (thread_count == 0U) {
        thread_count = std::max<std::size_t>(1U, std::thread::hardware_concurrency());
    }
    thread_count = std::min(thread_count, std::max<std::size_t>(1U, prefix_count));

    std::atomic<std::size_t> next_prefix{0U};
    std::vector<std::vector<ForwardCollisionPair>> per_thread(thread_count);
    std::vector<std::thread> workers;
    workers.reserve(thread_count);

    for (std::size_t worker = 0; worker < thread_count; ++worker) {
        workers.emplace_back([&, worker] {
            auto& output = per_thread[worker];
            while (true) {
                const auto prefix = next_prefix.fetch_add(1U, std::memory_order_relaxed);
                if (prefix >= prefix_count) {
                    break;
                }
                const auto bytes = prefix_bytes(static_cast<std::uint32_t>(prefix));
                const auto start = forward_state_for_research(bytes, parameters);
                const auto aliases = find_symbol_aliases(start, parameters);
                for (const auto& alias : aliases) {
                    output.push_back(ForwardCollisionPair{
                        .left = {bytes[0], bytes[1], alias.left},
                        .right = {bytes[0], bytes[1], alias.right},
                        .common_state = alias.common_state,
                        .differing_index = 2U,
                        .inherited = false,
                    });
                }
            }
        });
    }
    for (auto& worker : workers) {
        worker.join();
    }

    std::vector<ForwardCollisionPair> result;
    std::size_t total = 0U;
    for (const auto& part : per_thread) {
        total += part.size();
    }
    result.reserve(total);
    for (auto& part : per_thread) {
        result.insert(result.end(),
                      std::make_move_iterator(part.begin()),
                      std::make_move_iterator(part.end()));
    }
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        if (a.left != b.left) return a.left < b.left;
        return a.right < b.right;
    });
    return result;
}

inline std::vector<ForwardCollisionPair> inherited_three_byte_aliases(
    const HashParameters& parameters) {
    constexpr std::array<std::array<std::uint8_t, 4>, 3> bases{{
        {{0x17U, 0x6fU, 0x17U, 0x99U}},
        {{0x25U, 0x1cU, 0x25U, 0x46U}},
        {{0xa2U, 0x6fU, 0xa2U, 0x99U}},
    }};

    std::vector<ForwardCollisionPair> result;
    result.reserve(3U * 256U);
    for (const auto& base : bases) {
        const std::array<std::uint8_t, 2> left_prefix{{base[0], base[1]}};
        const std::array<std::uint8_t, 2> right_prefix{{base[2], base[3]}};
        const auto left_state = forward_state_for_research(left_prefix, parameters);
        const auto right_state = forward_state_for_research(right_prefix, parameters);
        if (left_state != right_state) {
            continue;
        }
        for (std::uint16_t third = 0; third < 256U; ++third) {
            const auto common = absorb_symbol_for_research(
                left_state, static_cast<std::uint8_t>(third), parameters);
            result.push_back(ForwardCollisionPair{
                .left = {base[0], base[1], static_cast<std::uint8_t>(third)},
                .right = {base[2], base[3], static_cast<std::uint8_t>(third)},
                .common_state = common,
                .differing_index = 1U,
                .inherited = true,
            });
        }
    }
    return result;
}

inline std::vector<ForwardCollisionPair> all_three_byte_forward_collisions(
    const HashParameters& parameters,
    std::size_t prefix_count = 65536U,
    std::size_t thread_count = 0U) {
    auto local = enumerate_local_three_byte_aliases(
        parameters, prefix_count, thread_count);
    if (prefix_count == 65536U) {
        auto inherited = inherited_three_byte_aliases(parameters);
        local.insert(local.end(),
                     std::make_move_iterator(inherited.begin()),
                     std::make_move_iterator(inherited.end()));
    }
    std::sort(local.begin(), local.end(), [](const auto& a, const auto& b) {
        if (a.left != b.left) return a.left < b.left;
        return a.right < b.right;
    });
    return local;
}

inline std::vector<std::uint8_t> as_vector(const std::array<std::uint8_t, 3>& value) {
    return {value.begin(), value.end()};
}

} // namespace pvc::tool
