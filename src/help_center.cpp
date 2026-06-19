#include "chh1dd3n3r/help_center.h"

namespace chh1dd3n3r {

static std::string colorize(const std::string& text, const char* code, bool use_color) {
    if (!use_color) return text;
    return std::string(code) + text + "\033[0m";
}

std::string HelpCenter::main_help(bool color) {
    auto C = [color](const std::string& txt, const char* code) {
        return colorize(txt, code, color);
    };
    return
        C("╭──────────────────────────────────────────────────────────────╮", "\033[36m") + "\n" +
        C("│  ChH1dd3n3r  │  Advanced File Steganography & Encryption      │", "\033[36m") + "\n" +
        C("╰──────────────────────────────────────────────────────────────╯", "\033[36m") + "\n" +
        C("USAGE:", "\033[37m") + "\n" +
        "  ChH1dd3n3r <COMMAND> [OPTIONS]\n\n" +
