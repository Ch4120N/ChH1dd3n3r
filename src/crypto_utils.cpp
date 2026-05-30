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
        if (1 != RAND_bytes(buffer.data(), static_cast<int>(count))) {
            throw ChH1dd3n3rError("Failed to generate secure random bytes.");
        }
    }
    return buffer;
}

std::vector<uint8_t> derive_key(const std::string& password,
