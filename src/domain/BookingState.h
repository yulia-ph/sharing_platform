#pragma once

#include <string>

#include "Enums.h"

namespace share {

class BookingState {
   public:
    virtual ~BookingState() = default;

    virtual bool isBusy() const = 0;
    virtual bool canPickup(const std::string& today, const std::string& startDate,
                           const std::string& endDate) const = 0;
    virtual bool canReturn() const = 0;
    virtual bool canCancel() const = 0;
    virtual bool becomesOverdue(const std::string& today, const std::string& endDate) const = 0;
};

const BookingState& bookingState(BookingStatus status);

}  // namespace share
