#include "BookingCharge.h"

#include <cmath>

#include "DateUtils.h"

namespace share {
namespace {

double roundKopecks(double value) {
    if (!std::isfinite(value) || value <= 0) return 0;
    return std::round(value * 100.0) / 100.0;
}

}  // namespace

double bookingCharge(BookingStatus status, const std::string& startDate, const std::string& endDate,
                     const std::string& today, double pricePerDay) {
    if (!(pricePerDay > 0) || today < startDate) return 0;

    double rate = 0;
    std::string until = today;
    if (status == BookingStatus::Reserved) {
        rate = 0.25;
        if (today > endDate) until = endDate;
    } else if (status == BookingStatus::Active) {
        rate = today > endDate ? 2.0 : 1.0;
    } else if (status == BookingStatus::Overdue) {
        rate = 2.0;
    } else {
        return 0;
    }

    const int days = inclusiveDays(startDate, until);
    return roundKopecks(pricePerDay * days * rate);
}

}  // namespace share
