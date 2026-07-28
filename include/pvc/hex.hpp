#pragma once

#include "pvc/hash.hpp"

#include <string>

namespace pvc {

[[nodiscard]] std::string to_hex(const Digest& digest);

} // namespace pvc
