#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chh1dd3n3r::crypto {

/**
 * @brief Generate cryptographically secure random bytes.
 */
std::vector<uint8_t> random_bytes(size_t count);

