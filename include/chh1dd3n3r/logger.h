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
    void error(const std::string& msg);
    void fatal(const std::string& msg);

    void grep(const std::string& key, const std::string& value);
    void json(const std::map<std::string, std::string>& data);

    // Allow direct access for Spinner (friend not needed)
    bool quiet_ = false;

private:
    bool color_;
    bool verbose_;
    std::string log_file_;
    std::mutex mutex_;

    std::string style(const std::string& text, const std::string& ansi_code) const;
    void log(const std::string& level, const std::string& msg,
             const std::string& ansi_code, bool to_stderr = false);
    void write_log_file(const std::string& level, const std::string& clean_line);
};

