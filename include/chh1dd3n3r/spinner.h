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
    ~Spinner();

    Spinner(const Spinner&) = delete;
    Spinner& operator=(const Spinner&) = delete;

    void start();
    void stop();

private:
    std::string text_;
    bool enabled_;
    std::atomic<bool> stop_flag_;
    std::thread thread_;

