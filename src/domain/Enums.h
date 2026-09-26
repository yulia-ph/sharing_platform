#pragma once

#include <string>

namespace share {

enum class BookingStatus {
    Reserved,
    Active,
    Returned,
    Cancelled,
};

std::string toString(BookingStatus status);

BookingStatus bookingStatusFromString(const std::string& s);

}