#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace chh1dd3n3r::crypto {

/**
 * @brief Generate cryptographically secure random bytes.
 */
std::vector<uint8_t> random_bytes(size_t count);

/**
 * @brief Derive a 32‑byte key using PBKDF2‑HMAC‑SHA256.
 */
std::vector<uint8_t> derive_key(const std::string& password,
                                const std::vector<uint8_t>& salt,
                                int iterations);

/**
 * @brief AES‑256‑GCM encrypt. Returns ciphertext || 16‑byte tag.
 */
std::vector<uint8_t> aes_gcm_encrypt(const std::vector<uint8_t>& key,
                                     const std::vector<uint8_t>& nonce,
                                     const std::vector<uint8_t>& plaintext,
                                     const std::vector<uint8_t>& aad);

/**
 * @brief AES‑256‑GCM decrypt. Expects ciphertext || tag.
 */
std::vector<uint8_t> aes_gcm_decrypt(const std::vector<uint8_t>& key,
                                     const std::vector<uint8_t>& nonce,
                                     const std::vector<uint8_t>& ciphertext_with_tag,
                                     const std::vector<uint8_t>& aad);

/**
 * @brief Encrypt a blob with a random salt/nonce.
 * Layout: salt (16) || nonce (12) || ciphertext || tag (16)
 */
std::vector<uint8_t> encrypt_blob(const std::string& password,
                                  const std::vector<uint8_t>& plaintext,
                                  const std::vector<uint8_t>& aad,
                                  int iterations);

/**
 * @brief Decrypt a blob produced by encrypt_blob.
 */
std::vector<uint8_t> decrypt_blob(const std::string& password,
                                  const std::vector<uint8_t>& blob,
                                  const std::vector<uint8_t>& aad,
                                  int iterations);

