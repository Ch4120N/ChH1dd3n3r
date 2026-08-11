#include <locale>

#include "chh1dd3n3r/cli.h"

int main(int argc, char* argv[]) {
    // Set UTF-8 locale
    std::locale::global(std::locale(""));

    // Run CLI
    return chh1dd3n3r::CLI::run(argc, argv);
