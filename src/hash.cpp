#include "pvc/hash.hpp"

#include "engine.hpp"

#include <bit>
#include <limits>
#include <stdexcept>

namespace pvc {

RotHash1::RotHash1() = default;

void RotHash1::update(std::span<const std::uint8_t> bytes) {
    if (bytes.size() > std::numeric_limits<std::size_t>::max() - message_.size()) {
        throw std::length_error("PVC-RotHash-1 message is too large");
    }
    message_.insert(message_.end(), bytes.begin(), bytes.end());
}

void RotHash1::update(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    update(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash1::finalize(bool keep_trace) const {
    const auto research = detail::compute(message_, canonical_hash_parameters(), keep_trace, false);

    HashResult result;
    if (research.digest.size() != result.digest.size()) {
        throw std::logic_error("canonical research digest has unexpected length");
    }
    for (std::size_t i = 0; i < result.digest.size(); ++i) {
        result.digest[i] = research.digest[i];
    }
    result.final_cube = research.final_state.cube;
    result.input_size = research.input_size;
    if (keep_trace) {
        result.trace = research.trace;
    }
    return result;
}

Digest RotHash1::hash(std::span<const std::uint8_t> bytes) {
    RotHash1 hasher;
    hasher.update(bytes);
    return hasher.finalize(false).digest;
}

Digest RotHash1::hash(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    return hash(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash1::inspect(std::span<const std::uint8_t> bytes, bool keep_trace) {
    RotHash1 hasher;
    hasher.update(bytes);
    return hasher.finalize(keep_trace);
}

RotHash2::RotHash2() = default;

void RotHash2::update(std::span<const std::uint8_t> bytes) {
    if (bytes.size() > std::numeric_limits<std::size_t>::max() - message_.size()) {
        throw std::length_error("PVC-RotHash-2 message is too large");
    }
    message_.insert(message_.end(), bytes.begin(), bytes.end());
}

void RotHash2::update(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    update(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash2::finalize(bool keep_trace) const {
    const auto research = detail::compute(message_, rothash2_hash_parameters(), keep_trace, false);

    HashResult result;
    if (research.digest.size() != result.digest.size()) {
        throw std::logic_error("rothash2 research digest has unexpected length");
    }
    for (std::size_t i = 0; i < result.digest.size(); ++i) {
        result.digest[i] = research.digest[i];
    }
    result.final_cube = research.final_state.cube;
    result.input_size = research.input_size;
    if (keep_trace) {
        result.trace = research.trace;
    }
    return result;
}

Digest RotHash2::hash(std::span<const std::uint8_t> bytes) {
    RotHash2 hasher;
    hasher.update(bytes);
    return hasher.finalize(false).digest;
}

Digest RotHash2::hash(std::string_view text) {
    const auto* begin = reinterpret_cast<const std::uint8_t*>(text.data());
    return hash(std::span<const std::uint8_t>{begin, text.size()});
}

HashResult RotHash2::inspect(std::span<const std::uint8_t> bytes, bool keep_trace) {
    RotHash2 hasher;
    hasher.update(bytes);
    return hasher.finalize(keep_trace);
}

std::size_t digest_hamming_distance(const Digest& left, const Digest& right) {
    std::size_t distance = 0;
    for (std::size_t i = 0; i < left.size(); ++i) {
        distance += static_cast<std::size_t>(
            std::popcount(static_cast<unsigned>(left[i] ^ right[i])));
    }
    return distance;
}

std::size_t digest_byte_distance(const Digest& left, const Digest& right) {
    std::size_t distance = 0;
    for (std::size_t i = 0; i < left.size(); ++i) {
        if (left[i] != right[i]) {
            ++distance;
        }
    }
    return distance;
}

} // namespace pvc
