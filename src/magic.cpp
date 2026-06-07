#include "chh1dd3n3r/magic.h"

#include <cstddef>

namespace chh1dd3n3r {

std::vector<uint8_t> xor_bytes(const std::vector<uint8_t>& data,
                               const std::vector<uint8_t>& key) {
    if (key.empty()) {
        return data;
