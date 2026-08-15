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
        C("COMMANDS:", "\033[37m") + "\n" +
        C("  hide      ", "\033[32m") + " Hide files/directories inside a carrier file\n" +
        C("  unhide    ", "\033[32m") + " Extract hidden files (alias: extract)\n" +
        C("  info      ", "\033[32m") + " Display container metadata (alias: list)\n" +
        C("  test      ", "\033[32m") + " Verify password can decrypt container\n" +
        C("  strip     ", "\033[32m") + " Remove hidden data, restore host file\n" +
        C("  shred     ", "\033[32m") + " Securely delete files\n" +
        C("  benchmark ", "\033[32m") + " Benchmark PBKDF2 speed\n" +
        C("  genkey    ", "\033[32m") + " Generate random key file\n\n" +
        C("GLOBAL OPTIONS:", "\033[37m") + "\n" +
        "  -h, --help            Show this help message\n" +
        "  --version             Show program version\n" +
        "  --no-color            Disable coloured output\n" +
        "  --no-banner           Suppress startup banner\n" +
        "  -v, --verbose         Verbose output\n" +
        "  -q, --quiet           Suppress non-error messages\n" +
        "  --grep                Machine-readable output\n" +
        "  --json                JSON output\n" +
        "  --log-file <PATH>     Write logs to file\n\n" +
        C("AUTHENTICATION OPTIONS:", "\033[37m") + "\n" +
        "  -p, --password <PWD>  Encryption/decryption password\n" +
        "  --key-file <PATH>     Read password from file\n" +
        "  --key-env <VAR>       Read password from environment variable\n\n" +
        C("EXAMPLES:", "\033[37m") + "\n" +
        "  ChH1dd3n3r hide -H photo.jpg -f secret.txt -o hidden.jpg -p MyP@ss\n" +
        "  ChH1dd3n3r unhide -i hidden.jpg -o ./recovered -p MyP@ss\n" +
        "  ChH1dd3n3r info hidden.jpg\n";
}

std::string HelpCenter::hide_help(bool color) {
    return
        "Usage:\n  ChH1dd3n3r hide [OPTIONS]\n\n"
        "Required:\n"
        "  -H, --host <FILE>      Carrier file\n"
        "  -o, --output <FILE>    Output container file\n"
        "  -f, --files <PATH>...  Files/directories to hide\n\n"
        "Authentication:\n"
        "  -p, --password <PWD>   Encryption password\n"
        "  --key-file <PATH>      Read password from file\n"
        "  --key-env <VAR>        Read password from environment variable\n\n"
        "Options:\n"
        "  --no-gzip              Disable GZip compression\n"
        "  --force                Overwrite output file if exists\n"
        "  --shred                Securely delete originals after hiding\n"
        "  --no-metadata          Do not preserve mtime/permissions\n"
        "  --pbkdf2-iterations <N> PBKDF2 iterations (default: 100000)\n";
}

std::string HelpCenter::unhide_help(bool color) {
    return
        "Usage:\n  ChH1dd3n3r unhide [OPTIONS]\n\n"
        "Required:\n"
        "  -i, --input <FILE>     Container file\n"
        "  -o, --output <DIR>     Output directory\n\n"
        "Authentication:\n"
        "  -p, --password <PWD>   Decryption password\n"
        "  --key-file <PATH>      Read password from file\n"
        "  --key-env <VAR>        Read password from environment variable\n\n"
        "Options:\n"
        "  --force                Overwrite existing files\n"
        "  --shred                Securely delete container after extraction\n"
        "  --extract-tar <POLICY> yes|no|ask (default: ask)\n"
        "  --pbkdf2-iterations <N> PBKDF2 iterations\n";
}

