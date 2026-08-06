#include "chh1dd3n3r/engine.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <zlib.h>

#include "chh1dd3n3r/container.h"
#include "chh1dd3n3r/crypto_utils.h"
#include "chh1dd3n3r/errors.h"
#include "chh1dd3n3r/magic.h"
#include "chh1dd3n3r/spinner.h"
#include "chh1dd3n3r/tar_utils.h"

namespace chh1dd3n3r {

namespace fs = std::filesystem;

namespace {

std::vector<uint8_t> gzip_compress(const std::vector<uint8_t>& data) {
    uLongf dest_len = compressBound(static_cast<uLong>(data.size()));
    std::vector<uint8_t> dest(dest_len);
    int ret = compress2(dest.data(), &dest_len, data.data(),
                        static_cast<uLong>(data.size()), Z_BEST_SPEED);
    if (ret != Z_OK) {
        throw ChH1dd3n3rError("GZip compression failed.");
    }
    dest.resize(dest_len);
    return dest;
}

std::vector<uint8_t> gzip_decompress(const std::vector<uint8_t>& data) {
    uLongf dest_len = static_cast<uLongf>(data.size() * 4 + 1024);
    while (true) {
        std::vector<uint8_t> dest(dest_len);
        int ret = uncompress(dest.data(), &dest_len, data.data(),
                             static_cast<uLong>(data.size()));
        if (ret == Z_OK) {
            dest.resize(dest_len);
            return dest;
        }
        if (ret == Z_BUF_ERROR) {
            dest_len *= 2;
            if (dest_len > 1ULL << 30) { // 1 GiB safety
                throw ChH1dd3n3rError("GZip decompression buffer too large.");
            }
            continue;
        }
        if (ret == Z_MEM_ERROR) {
