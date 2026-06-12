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
    std::vector<uint8_t> meta_salt;
    std::vector<uint8_t> meta_nonce;
};

/**
 * @brief Parse footer at a given position.
 */
FooterInfo parse_footer(const std::vector<uint8_t>& data, uint64_t footer_pos);

/**
 * @brief Decrypt metadata block.
 */
std::vector<uint8_t> decrypt_metadata(const std::vector<uint8_t>& encrypted_meta,
                                      const std::vector<uint8_t>& meta_salt,
                                      const std::vector<uint8_t>& meta_nonce,
                                      const std::string& password,
                                      int iterations);

/**
 * @brief Parsed metadata information.
 */
struct MetadataInfo {
    uint8_t version = 0;
    uint8_t flags = 0;
