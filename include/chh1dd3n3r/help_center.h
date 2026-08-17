#pragma once

#include <string>

namespace chh1dd3n3r {

/**
 * @brief Centralised help texts for all commands.
 */
class HelpCenter {
public:
    static std::string main_help(bool color);
    static std::string hide_help(bool color);
    static std::string unhide_help(bool color);
    static std::string info_help(bool color);
    static std::string test_help(bool color);
    static std::string strip_help(bool color);
    static std::string shred_help(bool color);
    static std::string benchmark_help(bool color);
    static std::string genkey_help(bool color);
};
