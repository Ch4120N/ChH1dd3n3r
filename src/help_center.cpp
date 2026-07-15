#include "chh1dd3n3r/help_center.h"

namespace chh1dd3n3r {

static std::string colorize(const std::string& text, const char* code, bool use_color) {
    if (!use_color) return text;
    return std::string(code) + text + "\033[0m";
}

std::string HelpCenter::main_help(bool color) {
    auto C = [color](const std::string& txt, const char* code) {
