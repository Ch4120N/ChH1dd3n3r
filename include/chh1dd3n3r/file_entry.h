#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chh1dd3n3r {

/**
 * @brief Represents a single file/directory before packing.
 */
struct FileEntry {
    std::string name;
    std::vector<uint8_t> data;
