#pragma once

#include <string>
#include <vector>

#include "chh1dd3n3r/logger.h"

namespace chh1dd3n3r {

/**
 * @brief Parses command line and runs the tool.
 */
class CLI {
public:
    static int run(int argc, char* argv[]);

private:
    struct Options;
    static void parse_args(int argc, char* argv[], Options& opts);
    static std::string resolve_password(const Options& opts);
    static std::string read_password_from_file(const std::string& path);
    static void print_banner(const Logger& logger);
};

