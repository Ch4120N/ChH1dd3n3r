#include "chh1dd3n3r/cli.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "chh1dd3n3r/engine.h"
#include "chh1dd3n3r/errors.h"
#include "chh1dd3n3r/help_center.h"
#include "chh1dd3n3r/logger.h"
#include "chh1dd3n3r/password_utils.h"

namespace chh1dd3n3r {

struct CLI::Options {
    std::string command;
