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
            if (!file) {
                throw InputFileError("Cannot read file: " + p.string());
            }
            std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                      std::istreambuf_iterator<char>());
            file.close();
            if (data.size() > 100ULL * 1024 * 1024) {
                logger_.warn("Large file detected (" +
                             std::to_string(data.size() / (1024 * 1024)) + " MB).");
            }
            if (gzip_compress) data = gzip_compress(data);

            auto ftime = fs::last_write_time(p);
            uint64_t mtime = static_cast<uint64_t>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    ftime.time_since_epoch()).count());
            auto perms = fs::status(p).permissions();
            collected.push_back({p.filename().string(),
                                 std::move(data),
                                 gzip_compress,
                                 mtime,
                                 static_cast<uint16_t>(perms)});
        } else {
            throw InputFileError("Path does not exist: " + p.string());
        }
    }

    std::vector<FileEntry> encrypted;
    encrypted.reserve(collected.size());
    for (size_t i = 0; i < collected.size(); ++i) {
        const auto& entry = collected[i];
        logger_.info("Encrypting " + std::to_string(i + 1) + "/" +
                     std::to_string(collected.size()) + ": " + entry.name +
                     " (" + std::to_string(entry.data.size()) + " bytes)");
        Spinner spinner("  Encrypting " + entry.name, !logger_.quiet_);
        spinner.start();
        std::vector<uint8_t> aad(entry.name.begin(), entry.name.end());
        std::vector<uint8_t> blob = crypto::encrypt_blob(password, entry.data,
                                                         aad, pbkdf2_iterations);
        encrypted.push_back({entry.name, std::move(blob), entry.is_gzip,
                             entry.mtime, entry.mode});
        spinner.stop();
    }

    std::vector<uint8_t> container_block =
        pack_v2_block(encrypted, gzip_compress, password, pbkdf2_iterations,
                      preserve_metadata);

    if (fs::exists(output_path) && !force) {
        throw OutputExistsError("Output '" + output_path + "' exists. Use --force to overwrite.");
    }

    std::ofstream out(output_path, std::ios::binary);
    if (!out) {
        throw ChH1dd3n3rError("Cannot write output file: " + output_path);
    }
    out.write(reinterpret_cast<const char*>(host_data.data()), host_data.size());
    out.write(reinterpret_cast<const char*>(container_block.data()),
              container_block.size());
    out.close();

    logger_.success("Hidden data written to '" + output_path + "' (" +
                    std::to_string(container_block.size()) + " bytes appended).");

    if (shred_originals) {
        shred(files, 3);
    }
}

void Engine::unhide(const std::string& input_path,
                    const std::string& output_dir,
                    const std::string& password,
                    bool force,
                    const std::string& extract_tar,
                    int pbkdf2_iterations,
                    bool shred_container) {
    if (!fs::is_regular_file(input_path)) {
        throw InputFileError("Container file not found: " + input_path);
    }

    std::ifstream input(input_path, std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());
