#include "chh1dd3n3r/container.h"

#include <cstring>
#include <stdexcept>

#include "chh1dd3n3r/errors.h"
#include "chh1dd3n3r/magic.h"
#include "chh1dd3n3r/crypto_utils.h"

namespace chh1dd3n3r {

namespace {

uint64_t read_be64(const uint8_t* p) {
    return (static_cast<uint64_t>(p[0]) << 56) |
           (static_cast<uint64_t>(p[1]) << 48) |
           (static_cast<uint64_t>(p[2]) << 40) |
           (static_cast<uint64_t>(p[3]) << 32) |
           (static_cast<uint64_t>(p[4]) << 24) |
           (static_cast<uint64_t>(p[5]) << 16) |
           (static_cast<uint64_t>(p[6]) << 8)  |
           static_cast<uint64_t>(p[7]);
}

void write_be64(std::vector<uint8_t>& vec, uint64_t value) {
    for (int i = 7; i >= 0; --i) {
        vec.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }
}

uint16_t read_be16(const uint8_t* p) {
    return static_cast<uint16_t>((p[0] << 8) | p[1]);
}

void write_be16(std::vector<uint8_t>& vec, uint16_t value) {
    vec.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    vec.push_back(static_cast<uint8_t>(value & 0xFF));
}

} // anonymous namespace

std::vector<uint8_t> pack_v2_block(const std::vector<FileEntry>& files,
                                   bool gzip_flag,
                                   const std::string& password,
                                   int iterations,
                                   bool preserve_metadata) {
    uint8_t flags = 0;
    if (gzip_flag) flags |= FLAG_GZIP;
    if (preserve_metadata) flags |= FLAG_METADATA;

    std::vector<uint8_t> meta_block;
    // Magic header v2
    meta_block.insert(meta_block.end(), MAGIC_HEADER_V2.begin(), MAGIC_HEADER_V2.end());
    meta_block.push_back(VERSION_V2);
    meta_block.push_back(flags);

    for (const auto& entry : files) {
        std::string name_utf8 = entry.name;
        write_be16(meta_block, static_cast<uint16_t>(name_utf8.size()));
        meta_block.insert(meta_block.end(), name_utf8.begin(), name_utf8.end());

        if (flags & FLAG_METADATA) {
            write_be64(meta_block, entry.mtime);
            write_be16(meta_block, entry.mode);
        }

        write_be64(meta_block, static_cast<uint64_t>(entry.data.size()));
        meta_block.insert(meta_block.end(), entry.data.begin(), entry.data.end());
    }

    // Encrypt metadata block
