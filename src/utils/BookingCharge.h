#pragma once

#include <string>

#include "Enums.h"

namespace share {

// Charge in BYN, rounded to kopecks. Nothing is counted before start.
// reserved: 0.25 through today, or through endDate when today is later.
// active and overdue: 1.0 from startDate through endDate, but not after today.
// Days after endDate through today are 2.0.
double bookingCharge(BookingStatus status, const std::string& startDate, const std::string& endDate,
                     const std::string& today, double pricePerDay);

}  // namespace share
