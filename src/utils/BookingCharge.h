#pragma once

#include <string>

#include "Enums.h"

namespace share {

// Charge in BYN, rounded to kopecks.
// reserved: 0.25 of the daily price, active: 1, overdue: 1.25. Days run from
// start through the last counted day, both included. Nothing is counted before start.
double bookingCharge(BookingStatus status, const std::string& startDate, const std::string& endDate,
                     const std::string& today, double pricePerDay);

}  // namespace share
