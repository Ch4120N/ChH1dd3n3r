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
    }
    fill_field(header.size, sizeof(header.size), std::to_string(size), true);

    uint64_t mtime = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::seconds>(
            fs::last_write_time(entry).time_since_epoch()).count());
    fill_field(header.mtime, sizeof(header.mtime), std::to_string(mtime), true);

    header.typeflag = is_dir ? '5' : '0';
    std::memcpy(header.magic, "ustar", 6);
    std::memcpy(header.version, "00", 2);
    fill_field(header.uname, sizeof(header.uname), "chh1dd3n3r");
    fill_field(header.gname, sizeof(header.gname), "chh1dd3n3r");
    fill_field(header.devmajor, sizeof(header.devmajor), "0", true);
    fill_field(header.devminor, sizeof(header.devminor), "0", true);

    set_checksum(header);
    append_bytes(archive, &header, sizeof(header));

    if (!is_dir && fs::is_regular_file(entry)) {
        std::ifstream file(entry, std::ios::binary);
        if (!file) {
            throw ChH1dd3n3rError("Failed to open file for tar: " + entry.string());
        }
        std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
        append_bytes(archive, buffer.data(), buffer.size());
        size_t pad = (BLOCK_SIZE - (buffer.size() % BLOCK_SIZE)) % BLOCK_SIZE;
        if (pad > 0) {
            archive.resize(archive.size() + pad, 0);
        }
    } else if (is_dir) {
        // no data for directory
    }
}

uint64_t parse_octal(const char* field, size_t size) {
    std::string str(field, size);
    // trim at first NUL or space
    size_t end = str.find_first_of("\0 ");
    if (end != std::string::npos) {
        str = str.substr(0, end);
    }
    if (str.empty()) {
        return 0;
    }
    try {
        return std::stoull(str, nullptr, 8);
    } catch (...) {
        throw MetadataError("Invalid octal field in tar header.");
    }
}

bool is_zero_block(const uint8_t* block) {
    for (size_t i = 0; i < BLOCK_SIZE; ++i) {
        if (block[i] != 0) {
            return false;
        }
    }
    return true;
}

std::string get_tar_path(const TarHeader& header) {
    std::string prefix(header.prefix, strnlen(header.prefix, sizeof(header.prefix)));
    std::string name(header.name, strnlen(header.name, sizeof(header.name)));
    if (!prefix.empty()) {
        return prefix + "/" + name;
    }
    return name;
}

} // anonymous namespace

std::vector<uint8_t> create_tar_from_directory(const fs::path& dir) {
    if (!fs::is_directory(dir)) {
        throw ChH1dd3n3rError("Path is not a directory: " + dir.string());
    }

    std::vector<uint8_t> archive;
    for (fs::recursive_directory_iterator it(dir), end; it != end; ++it) {
        const fs::path& entry = it->path();
        bool is_dir = fs::is_directory(entry);
        write_tar_entry(archive, dir, entry, is_dir);
    }
    append_zero_block(archive);
    append_zero_block(archive);
    return archive;
}

void extract_tar_to_directory(const std::vector<uint8_t>& tar_data,
                              const fs::path& output_dir) {
    if (tar_data.size() % BLOCK_SIZE != 0) {
        throw MetadataError("Tar data size is not a multiple of 512.");
    }

    fs::create_directories(output_dir);

    size_t offset = 0;
    bool prev_zero = false;
    std::string long_name;

    while (offset + BLOCK_SIZE <= tar_data.size()) {
        const uint8_t* block = tar_data.data() + offset;

        if (is_zero_block(block)) {
            if (prev_zero) break;
            prev_zero = true;
            offset += BLOCK_SIZE;
            continue;
        }
        prev_zero = false;

        TarHeader header;
        std::memcpy(&header, block, sizeof(TarHeader));
        offset += BLOCK_SIZE;

        uint64_t size = parse_octal(header.size, sizeof(header.size));

        if (header.typeflag == 'L') {
            // GNU long name
            if (offset + size > tar_data.size()) {
                throw MetadataError("Truncated long name in tar.");
            }
            long_name.assign(reinterpret_cast<const char*>(tar_data.data() + offset), size);
            long_name = long_name.substr(0, long_name.find('\0'));
            offset += ((size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
            continue;
        }

        std::string member = long_name.empty() ? get_tar_path(header) : long_name;
        long_name.clear();

        if (header.typeflag == '2') {
            // symlink – skip for safety
            offset += ((size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
            continue;
        }

        if (member.find("..") != std::string::npos || fs::path(member).is_absolute()) {
            throw MetadataError("Dangerous tar path rejected: " + member);
        }

        fs::path dest = output_dir / member;

        if (header.typeflag == '5' || header.typeflag == '\0' && size == 0) {
            fs::create_directories(dest);
            continue;
        }

        if (header.typeflag != '0' && header.typeflag != '\0') {
            // unknown type – skip data
            offset += ((size + BLOCK_SIZE - 1) / BLOCK_SIZE) * BLOCK_SIZE;
            continue;
        }

        fs::create_directories(dest.parent_path());
        if (offset + size > tar_data.size()) {
            throw MetadataError("Truncated tar file data.");
        }

        std::ofstream out(dest, std::ios::binary);
        if (!out) {
            throw ChH1dd3n3rError("Failed to create file during tar extraction: " + dest.string());
        }
        out.write(reinterpret_cast<const char*>(tar_data.data() + offset), size);
        out.close();

        // apply permissions (optional)
        uint64_t mode = parse_octal(header.mode, sizeof(header.mode));
        if (mode != 0) {
            fs::permissions(dest, static_cast<fs::perms>(mode & 0777),
                            fs::perm_options::replace);
        }

