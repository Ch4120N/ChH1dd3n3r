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
extern const std::vector<uint8_t> MAGIC_HEADER_V2;
extern const std::vector<uint8_t> MAGIC_FOOTER;

constexpr uint8_t VERSION_V1 = 1;
constexpr uint8_t VERSION_V2 = 2;
constexpr uint8_t FLAG_GZIP = 0x01;
constexpr uint8_t FLAG_METADATA = 0x02;

}