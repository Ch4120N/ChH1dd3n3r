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
                                const std::vector<uint8_t>& salt,
                                int iterations) {
    if (iterations <= 0) {
        throw ChH1dd3n3rError("PBKDF2 iterations must be > 0.");
    }

    std::vector<uint8_t> key(32);
    if (1 != PKCS5_PBKDF2_HMAC(
                    password.c_str(),
                    static_cast<int>(password.size()),
                    salt.data(),
                    static_cast<int>(salt.size()),
                    iterations,
                    EVP_sha256(),
                    32,
                    key.data())) {
        throw ChH1dd3n3rError("PBKDF2 key derivation failed.");
    }
    return key;
}

std::vector<uint8_t> aes_gcm_encrypt(const std::vector<uint8_t>& key,
                                     const std::vector<uint8_t>& nonce,
                                     const std::vector<uint8_t>& plaintext,
                                     const std::vector<uint8_t>& aad) {
    if (key.size() != 32) {
        throw ChH1dd3n3rError("AES-256-GCM requires a 32-byte key.");
    }
    if (nonce.size() != 12) {
        throw ChH1dd3n3rError("AES-256-GCM requires a 12-byte nonce.");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw ChH1dd3n3rError("Failed to create cipher context.");
    }

    std::vector<uint8_t> ciphertext(plaintext.size());
    std::vector<uint8_t> tag(16);
    int len = 0;
    int ciphertext_len = 0;

    auto cleanup = [&]() { EVP_CIPHER_CTX_free(ctx); };

    try {
        if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) {
            throw ChH1dd3n3rError("EVP_EncryptInit_ex failed.");
        }
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                                     static_cast<int>(nonce.size()), nullptr)) {
            throw ChH1dd3n3rError("Failed to set IV length.");
        }
        if (1 != EVP_EncryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data())) {
            throw ChH1dd3n3rError("Failed to set key/IV.");
        }

        if (!aad.empty()) {
            int aad_len = 0;
            if (1 != EVP_EncryptUpdate(ctx, nullptr, &aad_len,
                                       aad.data(), static_cast<int>(aad.size()))) {
                throw ChH1dd3n3rError("Failed to process AAD.");
            }
        }

        if (!plaintext.empty()) {
            if (1 != EVP_EncryptUpdate(ctx, ciphertext.data(), &len,
                                       plaintext.data(), static_cast<int>(plaintext.size()))) {
                throw ChH1dd3n3rError("Encryption failed.");
            }
            ciphertext_len = len;
        }

        if (1 != EVP_EncryptFinal_ex(ctx, ciphertext.data() + len, &len)) {
            throw ChH1dd3n3rError("Encryption finalisation failed.");
        }
        ciphertext_len += len;

        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag.data())) {
            throw ChH1dd3n3rError("Failed to get GCM tag.");
        }

        ciphertext.resize(ciphertext_len);
        ciphertext.insert(ciphertext.end(), tag.begin(), tag.end());

        cleanup();
        return ciphertext;
    } catch (...) {
        cleanup();
        throw;
    }
}

std::vector<uint8_t> aes_gcm_decrypt(const std::vector<uint8_t>& key,
                                     const std::vector<uint8_t>& nonce,
                                     const std::vector<uint8_t>& ciphertext_with_tag,
                                     const std::vector<uint8_t>& aad) {
    if (key.size() != 32) {
        throw ChH1dd3n3rError("AES-256-GCM requires a 32-byte key.");
    }
    if (nonce.size() != 12) {
        throw ChH1dd3n3rError("AES-256-GCM requires a 12-byte nonce.");
    }
    if (ciphertext_with_tag.size() < 16) {
        throw InvalidPasswordError("Ciphertext is too short (missing tag).");
    }

    size_t ct_len = ciphertext_with_tag.size() - 16;
    std::vector<uint8_t> ciphertext(ciphertext_with_tag.begin(),
                                    ciphertext_with_tag.begin() + ct_len);
    std::vector<uint8_t> tag(ciphertext_with_tag.end() - 16,
                             ciphertext_with_tag.end());

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw ChH1dd3n3rError("Failed to create cipher context.");
    }

    std::vector<uint8_t> plaintext(ct_len > 0 ? ct_len : 1);
    int len = 0;
    int plaintext_len = 0;

    auto cleanup = [&]() { EVP_CIPHER_CTX_free(ctx); };

    try {
        if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) {
            throw ChH1dd3n3rError("EVP_DecryptInit_ex failed.");
        }
        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN,
                                     static_cast<int>(nonce.size()), nullptr)) {
            throw ChH1dd3n3rError("Failed to set IV length.");
        }
        if (1 != EVP_DecryptInit_ex(ctx, nullptr, nullptr, key.data(), nonce.data())) {
            throw ChH1dd3n3rError("Failed to set key/IV.");
        }

        if (!aad.empty()) {
            int aad_len = 0;
            if (1 != EVP_DecryptUpdate(ctx, nullptr, &aad_len,
                                       aad.data(), static_cast<int>(aad.size()))) {
                throw ChH1dd3n3rError("Failed to process AAD.");
            }
        }

        if (!ciphertext.empty()) {
            if (1 != EVP_DecryptUpdate(ctx, plaintext.data(), &len,
                                       ciphertext.data(), static_cast<int>(ciphertext.size()))) {
                throw InvalidPasswordError("Decryption failed.");
            }
            plaintext_len = len;
        }

        if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag.data())) {
            throw ChH1dd3n3rError("Failed to set GCM tag.");
        }

        int ret = EVP_DecryptFinal_ex(ctx, plaintext.data() + len, &len);
        if (ret != 1) {
            throw InvalidPasswordError("Wrong password or corrupted ciphertext.");
        }
        plaintext_len += len;

        plaintext.resize(plaintext_len);
        cleanup();
        return plaintext;
    } catch (const InvalidPasswordError&) {
        cleanup();
        throw;
    } catch (...) {
        cleanup();
        throw ChH1dd3n3rError("Unexpected decryption error.");
    }
}

std::vector<uint8_t> encrypt_blob(const std::string& password,
                                  const std::vector<uint8_t>& plaintext,
                                  const std::vector<uint8_t>& aad,
                                  int iterations) {
    std::vector<uint8_t> salt = random_bytes(16);
    std::vector<uint8_t> key = derive_key(password, salt, iterations);
    std::vector<uint8_t> nonce = random_bytes(12);
    std::vector<uint8_t> encrypted = aes_gcm_encrypt(key, nonce, plaintext, aad);

    std::vector<uint8_t> blob;
