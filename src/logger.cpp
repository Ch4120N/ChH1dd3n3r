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
    }

    std::string prefix = "[*]";
    if (level == "SUCCESS") prefix = "[+]";
    else if (level == "WARN") prefix = "[-]";
    else if (level == "ERROR") prefix = "[!]";
    else if (level == "FATAL") prefix = "[X]";

    std::string line = style(prefix, ansi_code) + " " + msg;

    if (to_stderr) {
        std::cerr << line << std::endl;
    } else {
        std::cout << line << std::endl;
    }

    if (!log_file_.empty()) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::ofstream out(log_file_, std::ios::app);
        if (out) {
            std::string clean_line = line;
            // remove ANSI escape sequences
            std::string::size_type pos;
            while ((pos = clean_line.find("\033[")) != std::string::npos) {
                std::string::size_type end = clean_line.find('m', pos);
                if (end == std::string::npos) break;
                clean_line.erase(pos, end - pos + 1);
            }
            time_t now = time(nullptr);
            char time_buf[32];
            std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S",
                          localtime(&now));
            out << time_buf << " [" << level << "] " << clean_line << std::endl;
