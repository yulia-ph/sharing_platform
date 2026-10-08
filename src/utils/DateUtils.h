#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace share {

std::string today();

bool isValidDate(const std::string& s);

}  // namespace share