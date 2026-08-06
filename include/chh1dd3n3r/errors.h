#pragma once

#include <stdexcept>
#include <string>

namespace chh1dd3n3r {

/**
 * @brief Base exception for all ChH1dd3n3r errors.
 */
class ChH1dd3n3rError : public std::runtime_error {
public:
    explicit ChH1dd3n3rError(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Thrown when decryption fails (wrong password or corrupted data).
 */
class InvalidPasswordError : public ChH1dd3n3rError {
public:
    explicit InvalidPasswordError(const std::string& message)
        : ChH1dd3n3rError(message) {}
};

/**
 * @brief Thrown when the container format is malformed.
 */
class ContainerError : public ChH1dd3n3rError {
public:
    explicit ContainerError(const std::string& message)
        : ChH1dd3n3rError(message) {}
};

/**
 * @brief Thrown when metadata parsing fails.
 */
class MetadataError : public ContainerError {
public:
    explicit MetadataError(const std::string& message)
        : ContainerError(message) {}
};

/**
 * @brief Thrown when an output file/directory already exists.
 */
class OutputExistsError : public ChH1dd3n3rError {
public:
    explicit OutputExistsError(const std::string& message)
        : ChH1dd3n3rError(message) {}
};

