#include "chh1dd3n3r/spinner.h"

#include <iostream>
#include <chrono>

namespace chh1dd3n3r {

Spinner::Spinner(const std::string& text, bool enabled)
    : text_(text), enabled_(enabled), stop_flag_(false) {}

Spinner::~Spinner() {
    stop();
}

void Spinner::start() {
    if (!enabled_) return;
    stop_flag_ = false;
    thread_ = std::thread(&Spinner::animate, this);
}

void Spinner::stop() {
