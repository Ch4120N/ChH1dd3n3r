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
    /**
     * @brief Command‑line options.
     */
    struct Options {
        std::string command;
        bool color = true;
        bool banner = true;
        bool verbose = false;
        bool quiet = false;
        bool grep = false;
        bool json = false;
        std::string log_file;

        // hide
        std::string host, output, input, file, output_dir;
        std::vector<std::string> files;
        std::string password, key_file, key_env;
        bool no_gzip = false, force = false, shred = false, no_metadata = false;
        int pbkdf2_iterations = 100000;

        // unhide
        std::string extract_tar = "ask";

        // strip
        std::string output_path;

        // shred
        int passes = 3;

        // benchmark
        int iterations = 100000;

        // genkey
        int length = 32;
    };

    static int run(int argc, char* argv[]);

private:
    static void parse_args(int argc, char* argv[], Options& opts);
    static std::string resolve_password(const Options& opts);
    static std::string read_password_from_file(const std::string& path);
    static void print_banner(const Logger& logger);
};

}