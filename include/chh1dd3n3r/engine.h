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
