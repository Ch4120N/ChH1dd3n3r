#include "chh1dd3n3r/tar_utils.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "chh1dd3n3r/errors.h"

namespace chh1dd3n3r::tar {

namespace fs = std::filesystem;

namespace {

constexpr size_t BLOCK_SIZE = 512;

#pragma pack(push, 1)
struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};
#pragma pack(pop)

static_assert(sizeof(TarHeader) == 512, "TarHeader must be 512 bytes");

std::string to_octal(uint64_t value, size_t width) {
    std::ostringstream oss;
    oss << std::oct << std::setw(static_cast<int>(width)) << std::setfill('0') << value;
    std::string str = oss.str();
    if (str.size() > width) {
        throw ChH1dd3n3rError("Tar field too large for octal representation.");
    }
    return str;
}

void fill_field(char* dest, size_t dest_size, const std::string& value, bool numeric = false) {
    std::memset(dest, 0, dest_size);
    if (numeric) {
        std::string oct = to_octal(std::stoull(value.empty() ? "0" : value), dest_size - 1);
        std::memcpy(dest, oct.c_str(), oct.size());
        dest[dest_size - 1] = '\0';
    } else {
        size_t len = std::min(value.size(), dest_size - 1);
        std::memcpy(dest, value.c_str(), len);
        dest[len] = '\0';
    }
}

void clear_header(TarHeader& header) {
    std::memset(&header, 0, sizeof(header));
}

void set_checksum(TarHeader& header) {
    std::memset(header.checksum, ' ', sizeof(header.checksum));
    uint64_t sum = 0;
    const unsigned char* raw = reinterpret_cast<const unsigned char*>(&header);
    for (size_t i = 0; i < sizeof(TarHeader); ++i) {
        sum += raw[i];
    }
    std::string chk = to_octal(sum, 6);
    std::memcpy(header.checksum, chk.c_str(), chk.size());
    header.checksum[6] = '\0';
    header.checksum[7] = ' ';
}

void append_bytes(std::vector<uint8_t>& vec, const void* data, size_t size) {
    const uint8_t* ptr = static_cast<const uint8_t*>(data);
    vec.insert(vec.end(), ptr, ptr + size);
}

void append_zero_block(std::vector<uint8_t>& vec) {
    vec.resize(vec.size() + BLOCK_SIZE, 0);
}

std::string relative_path(const fs::path& root, const fs::path& entry) {
    return fs::relative(entry, root).generic_string();
}

void split_tar_name(const std::string& full, std::string& name, std::string& prefix) {
    if (full.size() <= 100) {
        name = full;
        prefix.clear();
        return;
    }
    // split into prefix (<=155) and name (<=100)
    size_t slash = full.find('/', full.size() - 100);
    while (slash != std::string::npos && slash > 155) {
        slash = full.find('/', slash - 1);
    }
    if (slash == std::string::npos || full.substr(0, slash).size() > 155 ||
        full.substr(slash + 1).size() > 100) {
        throw ChH1dd3n3rError("Tar path too long: " + full);
    }
    prefix = full.substr(0, slash);
    name = full.substr(slash + 1);
}

void write_tar_entry(std::vector<uint8_t>& archive, const fs::path& root,
                     const fs::path& entry, bool is_dir) {
    TarHeader header;
    clear_header(header);

    std::string rel = relative_path(root, entry);
    if (is_dir && !rel.empty() && rel.back() != '/') {
        rel += '/';
    }

    std::string name, prefix;
    split_tar_name(rel, name, prefix);
    fill_field(header.name, sizeof(header.name), name);
    fill_field(header.prefix, sizeof(header.prefix), prefix);

    auto st = fs::symlink_status(entry);
    uint32_t mode = static_cast<uint32_t>(st.permissions());
    fill_field(header.mode, sizeof(header.mode), std::to_string(mode), true);
    fill_field(header.uid, sizeof(header.uid), "0", true);
    fill_field(header.gid, sizeof(header.gid), "0", true);

    uint64_t size = 0;
    if (!is_dir && fs::is_regular_file(entry)) {
        size = fs::file_size(entry);
