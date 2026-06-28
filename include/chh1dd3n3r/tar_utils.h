#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace chh1dd3n3r::tar {

/**
 * @brief Create a ustar tar archive from a directory (recursive).
 */
std::vector<uint8_t> create_tar_from_directory(const std::filesystem::path& dir);

/**
 * @brief Extract a ustar tar archive to an output directory.
