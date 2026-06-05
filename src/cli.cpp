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
    int passes = 3;

    // benchmark
    int iterations = 100000;

    // genkey
    int length = 32;
};

namespace {

bool has_flag(const std::vector<std::string>& args, const std::string& flag) {
    for (const auto& a : args) {
        if (a == flag) return true;
    }
    return false;
}

std::string get_value_after(const std::vector<std::string>& args,
                            const std::string& flag,
                            size_t& i) {
    if (i + 1 < args.size()) {
        return args[++i];
    }
    throw ChH1dd3n3rError("Missing value for option: " + flag);
}

void parse_common(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--no-color") opts.color = false;
        else if (a == "--no-banner") opts.banner = false;
        else if (a == "-v" || a == "--verbose") opts.verbose = true;
        else if (a == "-q" || a == "--quiet") opts.quiet = true;
        else if (a == "--grep") opts.grep = true;
        else if (a == "--json") opts.json = true;
        else if (a == "--log-file") opts.log_file = get_value_after(args, a, i);
    }
}

void parse_hide(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-H" || a == "--host") opts.host = get_value_after(args, a, i);
        else if (a == "-o" || a == "--output") opts.output = get_value_after(args, a, i);
        else if (a == "-f" || a == "--files") {
            ++i;
            while (i < args.size() && args[i][0] != '-') {
                opts.files.push_back(args[i++]);
            }
            --i; // compensate for loop increment
        }
        else if (a == "-p" || a == "--password") opts.password = get_value_after(args, a, i);
        else if (a == "--key-file") opts.key_file = get_value_after(args, a, i);
        else if (a == "--key-env") opts.key_env = get_value_after(args, a, i);
        else if (a == "--no-gzip") opts.no_gzip = true;
        else if (a == "--force") opts.force = true;
        else if (a == "--shred") opts.shred = true;
        else if (a == "--no-metadata") opts.no_metadata = true;
        else if (a == "--pbkdf2-iterations")
            opts.pbkdf2_iterations = std::stoi(get_value_after(args, a, i));
    }
}

void parse_unhide(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-i" || a == "--input") opts.input = get_value_after(args, a, i);
        else if (a == "-o" || a == "--output") opts.output_dir = get_value_after(args, a, i);
        else if (a == "-p" || a == "--password") opts.password = get_value_after(args, a, i);
        else if (a == "--key-file") opts.key_file = get_value_after(args, a, i);
        else if (a == "--key-env") opts.key_env = get_value_after(args, a, i);
        else if (a == "--force") opts.force = true;
        else if (a == "--shred") opts.shred = true;
        else if (a == "--extract-tar") opts.extract_tar = get_value_after(args, a, i);
        else if (a == "--pbkdf2-iterations")
            opts.pbkdf2_iterations = std::stoi(get_value_after(args, a, i));
    }
}

void parse_info(const std::vector<std::string>& args, CLI::Options& opts) {
    // positional file
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--password") opts.password = get_value_after(args, a, i);
        else if (a == "--key-file") opts.key_file = get_value_after(args, a, i);
        else if (a == "--key-env") opts.key_env = get_value_after(args, a, i);
        else if (a == "--pbkdf2-iterations")
            opts.pbkdf2_iterations = std::stoi(get_value_after(args, a, i));
        else if (a[0] != '-') opts.file = a;
    }
}

void parse_test(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-i" || a == "--input") opts.input = get_value_after(args, a, i);
        else if (a == "-p" || a == "--password") opts.password = get_value_after(args, a, i);
        else if (a == "--key-file") opts.key_file = get_value_after(args, a, i);
        else if (a == "--key-env") opts.key_env = get_value_after(args, a, i);
        else if (a == "--pbkdf2-iterations")
            opts.pbkdf2_iterations = std::stoi(get_value_after(args, a, i));
    }
}

void parse_strip(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "-i" || a == "--input") opts.input = get_value_after(args, a, i);
        else if (a == "-o" || a == "--output") opts.output_path = get_value_after(args, a, i);
        else if (a == "--force") opts.force = true;
    }
}

void parse_shred(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
