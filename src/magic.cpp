#include "chh1dd3n3r/magic.h"

#include <cstddef>

namespace chh1dd3n3r {

std::vector<uint8_t> xor_bytes(const std::vector<uint8_t>& data,
                               const std::vector<uint8_t>& key) {
    if (key.empty()) {
        return data;
    }
    std::vector<uint8_t> result;
    result.reserve(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        result.push_back(data[i] ^ key[i % key.size()]);
    }
    return result;
}

// XOR‑obfuscated values – the literal magic strings never appear in source.
static const std::vector<uint8_t> MAGIC_HEADER_V1_XORED = {
    0x23, 0xF8, 0x84, 0x95, 0xAA
};
static const std::vector<uint8_t> MAGIC_HEADER_V2_XORED = {
    0x23, 0xF8, 0x84, 0x95, 0x9B
};
static const std::vector<uint8_t> MAGIC_FOOTER_XORED = {
    0xE9, 0xE2, 0xE2, 0xEE, 0xEF, 0xE4, 0xEE
};

static const std::vector<uint8_t> MAGIC_XOR_KEY = {
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE
};
static const std::vector<uint8_t> MAGIC_FOOTER_XOR_KEY = {
