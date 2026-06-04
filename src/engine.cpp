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
            throw ChH1dd3n3rError("Out of memory during GZip decompression.");
        }
        throw ChH1dd3n3rError("GZip decompression failed.");
    }
}

bool is_tar_data(const std::vector<uint8_t>& data) {
    return tar::is_tar(data);
}

} // anonymous namespace

Engine::Engine(Logger& logger) : logger_(logger) {}

void Engine::hide(const std::string& host_path,
                  const std::string& output_path,
                  const std::vector<std::string>& files,
                  const std::string& password,
                  bool gzip_compress,
                  bool force,
                  int pbkdf2_iterations,
                  bool preserve_metadata,
                  bool shred_originals) {
    if (!fs::is_regular_file(host_path)) {
        throw InputFileError("Host file not found: " + host_path);
    }

    std::ifstream host_file(host_path, std::ios::binary);
    if (!host_file) {
        throw InputFileError("Cannot read host file: " + host_path);
    }
    std::vector<uint8_t> host_data((std::istreambuf_iterator<char>(host_file)),
                                   std::istreambuf_iterator<char>());
    host_file.close();

    std::vector<FileEntry> collected;
    for (const auto& path_str : files) {
        fs::path p(path_str);
        if (fs::is_directory(p)) {
            logger_.info("Packing directory: " + p.string());
            std::vector<uint8_t> tar_data = tar::create_tar_from_directory(p);
            if (gzip_compress) tar_data = gzip_compress(tar_data);

            auto ftime = fs::last_write_time(p);
            uint64_t mtime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    ftime.time_since_epoch()).count());
            auto perms = fs::status(p).permissions();
            collected.push_back({p.filename().string() + ".tar",
                                 std::move(tar_data),
                                 gzip_compress,
                                 mtime,
                                 static_cast<uint16_t>(perms)});
        } else if (fs::is_regular_file(p)) {
            std::ifstream file(p, std::ios::binary);
