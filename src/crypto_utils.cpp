#include "chh1dd3n3r/crypto_utils.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/pkcs5.h>
#include <openssl/sha.h>

#include <stdexcept>
#include <cstring>

#include "chh1dd3n3r/errors.h"

namespace chh1dd3n3r::crypto {

std::vector<uint8_t> random_bytes(size_t count) {
    std::vector<uint8_t> buffer(count);
    if (count > 0) {
