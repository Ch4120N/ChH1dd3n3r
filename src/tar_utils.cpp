#include "chh1dd3n3r/tar_utils.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "chh1dd3n3r/errors.h"

namespace chh1dd3n3r::tar {

namespace fs = std::filesystem;

namespace {

constexpr size_t BLOCK_SIZE = 512;
