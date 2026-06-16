#include "chh1dd3n3r/logger.h"

#include <iostream>
#include <sstream>
#include <ctime>

namespace chh1dd3n3r {

Logger::Logger(bool color, bool verbose, bool quiet, const std::string& log_file)
    : quiet_(quiet), color_(color), verbose_(verbose), log_file_(log_file) {}

void Logger::set_color(bool enabled) { color_ = enabled; }
void Logger::set_verbose(bool enabled) { verbose_ = enabled; }
