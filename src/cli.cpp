#include "chh1dd3n3r/cli.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "chh1dd3n3r/engine.h"
#include "chh1dd3n3r/errors.h"
#include "chh1dd3n3r/help_center.h"
#include "chh1dd3n3r/logger.h"
#include "chh1dd3n3r/password_utils.h"

namespace chh1dd3n3r {

struct CLI::Options {
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
