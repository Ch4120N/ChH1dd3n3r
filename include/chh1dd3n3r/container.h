#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "chh1dd3n3r/file_entry.h"

namespace chh1dd3n3r {

/**
 * @brief Pack files into an encrypted v2 container block.
 * Returns encrypted_metadata || footer.
 */
std::vector<uint8_t> pack_v2_block(const std::vector<FileEntry>& files,
                                   bool gzip_flag,
                                   const std::string& password,
                                   int iterations,
                                   bool preserve_metadata);

/**
 * @brief Parsed footer information.
 */
struct FooterInfo {
    uint64_t enc_len = 0;
