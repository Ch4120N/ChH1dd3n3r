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
    input.close();

    auto footer_it = std::search(data.rbegin(), data.rend(),
                                 MAGIC_FOOTER.rbegin(), MAGIC_FOOTER.rend());
    if (footer_it == data.rend()) {
        throw ContainerError("No valid ChH1dd3n3r container footer found.");
    }
    uint64_t footer_pos = static_cast<uint64_t>(data.size() -
                                                std::distance(data.rbegin(), footer_it) -
                                                MAGIC_FOOTER.size());

    FooterInfo footer = parse_footer(data, footer_pos);
    if (footer.enc_len > footer_pos) {
        throw ContainerError("Invalid container layout (negative metadata offset).");
    }
    uint64_t encrypted_meta_start = footer_pos - footer.enc_len;
    std::vector<uint8_t> encrypted_meta(data.begin() + encrypted_meta_start,
                                        data.begin() + footer_pos);

    std::vector<uint8_t> plain_meta =
        decrypt_metadata(encrypted_meta, footer.meta_salt, footer.meta_nonce,
                         password, pbkdf2_iterations);

    MetadataInfo info = parse_metadata_block(plain_meta);

    logger_.info("Container version " + std::to_string(info.version) +
                 ", gzip=" + ((info.flags & FLAG_GZIP) ? "yes" : "no") +
                 ", metadata=" + ((info.flags & FLAG_METADATA) ? "yes" : "no"));

    if (info.files.empty()) {
        logger_.warn("Container is empty.");
        return;
    }

    fs::create_directories(output_dir);

    for (const auto& file : info.files) {
        logger_.info("Extracting: " + file.name);
        if (file.data_offset + file.data_len > plain_meta.size()) {
            throw MetadataError("File data offset out of range.");
        }
        std::vector<uint8_t> enc_blob(plain_meta.begin() + file.data_offset,
                                      plain_meta.begin() + file.data_offset + file.data_len);

        Spinner spinner("  Decrypting " + file.name, !logger_.quiet_);
        spinner.start();
        std::vector<uint8_t> plain = crypto::decrypt_blob(password, enc_blob,
                                                          std::vector<uint8_t>(file.name.begin(), file.name.end()),
                                                          pbkdf2_iterations);
        spinner.stop();

        if (info.flags & FLAG_GZIP) {
            try {
                plain = gzip_decompress(plain);
            } catch (...) {
                logger_.warn("GZip decompression failed for " + file.name +
                             ". Saving raw data.");
            }
        }

        if (tar::is_tar(plain)) {
            bool do_extract = false;
            if (extract_tar == "yes") do_extract = true;
            else if (extract_tar == "no") do_extract = false;
            else {
                std::cout << "  [?] " << file.name <<
                             " appears to be a directory (tar). Extract contents? [Y/n] ";
                std::string answer;
                std::getline(std::cin, answer);
                std::transform(answer.begin(), answer.end(), answer.begin(),
                               [](unsigned char c) { return std::tolower(c); });
                do_extract = answer.empty() || answer == "y" || answer == "yes";
            }
            if (do_extract) {
                logger_.info("Extracting directory contents from " + file.name + " ...");
                try {
                    tar::extract_tar_to_directory(plain, output_dir);
                    logger_.success("Extracted directory tree to " + output_dir);
                } catch (const std::exception& e) {
                    logger_.error("Failed to extract tar: " + std::string(e.what()));
                }
            }
            continue;
        }

        fs::path dest = fs::path(output_dir) / file.name;
        if (fs::exists(dest) && !force) {
            throw OutputExistsError("'" + dest.string() + "' already exists. Use --force to overwrite.");
        }

        std::ofstream out(dest, std::ios::binary);
        if (!out) {
            throw ChH1dd3n3rError("Cannot write file: " + dest.string());
        }
        out.write(reinterpret_cast<const char*>(plain.data()), plain.size());
        out.close();

        apply_metadata(dest, file.mtime, file.mode);
        logger_.success("Written " + std::to_string(plain.size()) + " bytes to " + dest.string());
    }

    if (shred_container) {
        shred({input_path}, 3);
    }
}

void Engine::info(const std::string& input_path,
                  const std::string& password,
                  bool grep,
                  bool json_output,
                  int pbkdf2_iterations) {
    if (!fs::is_regular_file(input_path)) {
        throw InputFileError("File not found: " + input_path);
    }

    std::ifstream input(input_path, std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());
    input.close();

    auto footer_it = std::search(data.rbegin(), data.rend(),
                                 MAGIC_FOOTER.rbegin(), MAGIC_FOOTER.rend());
    if (footer_it == data.rend()) {
        throw ContainerError("No container footer found.");
    }
    uint64_t footer_pos = static_cast<uint64_t>(data.size() -
                                                std::distance(data.rbegin(), footer_it) -
                                                MAGIC_FOOTER.size());
    FooterInfo footer = parse_footer(data, footer_pos);
    std::vector<uint8_t> encrypted_meta(data.begin() + footer_pos - footer.enc_len,
                                        data.begin() + footer_pos);

    if (password.empty()) {
        if (json_output) {
            logger_.json({{"status", "encrypted_metadata"}});
        } else if (grep) {
            logger_.grep("STATUS", "ENCRYPTED_METADATA");
        } else {
            std::cout << "╔════════════════════════════════════════════╗\n"
                         "║ Encrypted Container – provide --password to list files.\n"
                         "╚════════════════════════════════════════════╝\n";
        }
        return;
    }

    std::vector<uint8_t> plain_meta =
        decrypt_metadata(encrypted_meta, footer.meta_salt, footer.meta_nonce,
                         password, pbkdf2_iterations);
    MetadataInfo info = parse_metadata_block(plain_meta);

    if (json_output) {
        std::map<std::string, std::string> out;
        out["version"] = std::to_string(info.version);
        out["gzip"] = (info.flags & FLAG_GZIP) ? "true" : "false";
        out["metadata_preserved"] = (info.flags & FLAG_METADATA) ? "true" : "false";
        out["files_count"] = std::to_string(info.files.size());
        size_t total = 0;
