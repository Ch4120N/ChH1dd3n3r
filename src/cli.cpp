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
        if (a == "--passes") opts.passes = std::stoi(get_value_after(args, a, i));
        else if (a[0] != '-') opts.files.push_back(a);
    }
}

void parse_benchmark(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--iterations") opts.iterations = std::stoi(get_value_after(args, a, i));
    }
}

void parse_genkey(const std::vector<std::string>& args, CLI::Options& opts) {
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        if (a == "--length") opts.length = std::stoi(get_value_after(args, a, i));
        else if (a == "--force") opts.force = true;
        else if (a[0] != '-') opts.output_path = a;
    }
}

} // anonymous namespace

void CLI::parse_args(int argc, char* argv[], Options& opts) {
    if (argc < 2) {
        std::cout << HelpCenter::main_help(true) << std::endl;
        exit(0);
    }

    std::string cmd = argv[1];
    if (cmd == "extract") cmd = "unhide";
    else if (cmd == "list") cmd = "info";
    opts.command = cmd;

    std::vector<std::string> args;
    for (int i = 2; i < argc; ++i) args.emplace_back(argv[i]);

    parse_common(args, opts);

    if (cmd == "hide") parse_hide(args, opts);
    else if (cmd == "unhide") parse_unhide(args, opts);
    else if (cmd == "info") parse_info(args, opts);
    else if (cmd == "test") parse_test(args, opts);
    else if (cmd == "strip") parse_strip(args, opts);
    else if (cmd == "shred") parse_shred(args, opts);
    else if (cmd == "benchmark") parse_benchmark(args, opts);
    else if (cmd == "genkey") parse_genkey(args, opts);
    else {
        std::cerr << "Unknown command: " << cmd << "\n\n";
        std::cout << HelpCenter::main_help(opts.color) << std::endl;
        exit(2);
    }
}

std::string CLI::read_password_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw ChH1dd3n3rError("Failed to read key file: " + path);
    }
    std::string pwd;
    std::getline(file, pwd);
    if (!pwd.empty() && pwd.back() == '\r') pwd.pop_back();
    return pwd;
}

std::string CLI::resolve_password(const Options& opts) {
    if (!opts.password.empty()) return opts.password;
    if (!opts.key_file.empty()) return read_password_from_file(opts.key_file);
    if (!opts.key_env.empty()) {
        const char* env = std::getenv(opts.key_env.c_str());
        if (env) return env;
        throw ChH1dd3n3rError("Environment variable '" + opts.key_env + "' is not set");
    }
    if (opts.command == "hide" || opts.command == "unhide" ||
        opts.command == "test" ||
        (opts.command == "info" && !opts.password.empty())) {
        return read_password("Password: ");
    }
    return "";
}

void CLI::print_banner(const Logger& logger) {
    std::cout <<
        "   ____ _   _    _ _   _  _               _____\n"
        "  / ___| | | |  / | ||_| ||_|  __ _  ___ |___ / _ __\n"
        " | |   | |_| |  | |__ | ||_   / _` |/ _ \\  |_ \\| '__|\n"
        " | |___|  _  |  | '_ \\| |__   | (_| |  __/ ___) | |\n"
        "  \\____|_| |_|  |_| |_|   |_|  \\__,_|\\___||____/|_|\n"
        "  File Steganography & Encryption v1.0  by Ch4120N\n\n";
}

int CLI::run(int argc, char* argv[]) {
#ifdef _WIN32
    // Set the console code page to UTF-8.
    // This is the programmatic equivalent of `chcp 65001`,
    // without invoking a shell command.
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    Options opts;
    parse_args(argc, argv, opts);

    Logger logger(opts.color, opts.verbose, opts.quiet, opts.log_file);

    if (opts.banner && !opts.quiet) {
        print_banner(logger);
    }

    Engine engine(logger);

    try {
        if (opts.command == "hide") {
            std::string password = resolve_password(opts);
