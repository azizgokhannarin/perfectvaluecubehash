#include "pvc/hex.hpp"

namespace pvc {
namespace {

constexpr char kHex[] = "0123456789abcdef";

template <typename Array>
std::string encode(const Array& bytes) {
    std::string result;
    result.resize(bytes.size() * 2U);

    std::size_t out = 0;
    for (const auto byte : bytes) {
        result[out++] = kHex[(byte >> 4U) & 0x0FU];
        result[out++] = kHex[byte & 0x0FU];
    }
    return result;
}

} // namespace

std::string to_hex(const Digest& digest) {
    return encode(digest);
}

} // namespace pvc
