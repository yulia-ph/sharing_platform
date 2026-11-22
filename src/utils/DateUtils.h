#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace share {

std::string today();
std::string tomorrow();

bool isValidDate(const std::string& s);

// Days from start through end, both included. Zero when end is earlier than start.
int inclusiveDays(const std::string& start, const std::string& end);

}  // namespace share