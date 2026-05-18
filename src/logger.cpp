#include "chh1dd3n3r/logger.h"

#include <iostream>
#include <sstream>
#include <ctime>

namespace chh1dd3n3r {

Logger::Logger(bool color, bool verbose, bool quiet, const std::string& log_file)
    : quiet_(quiet), color_(color), verbose_(verbose), log_file_(log_file) {}

void Logger::set_color(bool enabled) { color_ = enabled; }
void Logger::set_verbose(bool enabled) { verbose_ = enabled; }
void Logger::set_quiet(bool enabled) { quiet_ = enabled; }
void Logger::set_log_file(const std::string& path) { log_file_ = path; }

std::string Logger::style(const std::string& text, const std::string& ansi_code) const {
    if (!color_) {
        return text;
    }
    return ansi_code + text + "\033[0m";
}

void Logger::log(const std::string& level, const std::string& msg,
                 const std::string& ansi_code, bool to_stderr) {
    if (quiet_ && level != "ERROR" && level != "FATAL") {
        return;
