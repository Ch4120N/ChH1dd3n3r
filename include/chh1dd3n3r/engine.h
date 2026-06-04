#pragma once

#include <string>
#include <vector>

#include "chh1dd3n3r/logger.h"
#include "chh1dd3n3r/file_entry.h"

namespace chh1dd3n3r {

/**
 * @brief Core engine for all commands.
 */
class Engine {
public:
    Engine(Logger& logger);

    void hide(const std::string& host_path,
              const std::string& output_path,
              const std::vector<std::string>& files,
              const std::string& password,
              bool gzip_compress,
              bool force,
              int pbkdf2_iterations,
              bool preserve_metadata,
              bool shred_originals);

    void unhide(const std::string& input_path,
                const std::string& output_dir,
                const std::string& password,
                bool force,
                const std::string& extract_tar,
                int pbkdf2_iterations,
                bool shred_container);

    void info(const std::string& input_path,
              const std::string& password,
              bool grep,
              bool json_output,
              int pbkdf2_iterations);

    bool test(const std::string& input_path,
