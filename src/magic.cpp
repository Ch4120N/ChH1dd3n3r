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
