#include "chh1dd3n3r/engine.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <zlib.h>

#include "chh1dd3n3r/container.h"
#include "chh1dd3n3r/crypto_utils.h"
#include "chh1dd3n3r/errors.h"
#include "chh1dd3n3r/magic.h"
#include "chh1dd3n3r/spinner.h"
#include "chh1dd3n3r/tar_utils.h"

namespace chh1dd3n3r {

namespace fs = std::filesystem;

namespace {

std::vector<uint8_t> gzip_compress(const std::vector<uint8_t>& data) {
    uLongf dest_len = compressBound(static_cast<uLong>(data.size()));
    std::vector<uint8_t> dest(dest_len);
    int ret = compress2(dest.data(), &dest_len, data.data(),
