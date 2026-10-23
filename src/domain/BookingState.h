#pragma once

#include "Enums.h"

namespace share {

class BookingState {
   public:
    virtual ~BookingState() = default;
    virtual bool isBusy() const = 0;
};

const BookingState& bookingState(BookingStatus status);

}  // namespace share
