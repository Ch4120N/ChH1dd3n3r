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
