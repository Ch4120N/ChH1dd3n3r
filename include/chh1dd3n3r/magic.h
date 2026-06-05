#pragma once

#include <cstdint>
#include <vector>

namespace chh1dd3n3r {

/**
 * @brief XOR two byte vectors with a repeating key.
 */
std::vector<uint8_t> xor_bytes(const std::vector<uint8_t>& data,
                               const std::vector<uint8_t>& key);

// Obfuscated magic constants – reconstructed at runtime.
extern const std::vector<uint8_t> MAGIC_HEADER_V1;
