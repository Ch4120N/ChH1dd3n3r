#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <map>

namespace chh1dd3n3r {

/**
 * @brief Console logging with ANSI colours and optional JSON/grep output.
 */
class Logger {
public:
    Logger(bool color = true, bool verbose = false, bool quiet = false,
           const std::string& log_file = "");

    void set_color(bool enabled);
    void set_verbose(bool enabled);
    void set_quiet(bool enabled);
    void set_log_file(const std::string& path);

    void info(const std::string& msg);
    void success(const std::string& msg);
    void warn(const std::string& msg);
