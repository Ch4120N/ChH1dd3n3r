#pragma once

#include <atomic>
#include <string>
#include <thread>

namespace chh1dd3n3r {

/**
 * @brief A simple thread‑based console spinner.
 */
class Spinner {
public:
    Spinner(const std::string& text, bool enabled = true);
