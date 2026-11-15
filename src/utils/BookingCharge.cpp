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

    if (status == BookingStatus::Reserved) {
        std::string until = today;
        if (today > endDate) until = endDate;
        const int days = inclusiveDays(startDate, until);
        return roundKopecks(pricePerDay * days * 0.25);
    }

    if (status != BookingStatus::Active && status != BookingStatus::Overdue) return 0;

    const std::string until = today < endDate ? today : endDate;
    const int regularDays = inclusiveDays(startDate, until);
    int lateDays = 0;
    if (today > endDate) lateDays = inclusiveDays(endDate, today) - 1;
    return roundKopecks(pricePerDay * regularDays + pricePerDay * lateDays * 2.0);
}

}  // namespace share
