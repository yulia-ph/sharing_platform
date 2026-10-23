#include "BookingState.h"

#include <stdexcept>

namespace share {
namespace {

class ReservedState : public BookingState {
   public:
    bool isBusy() const override { return true; }
};

class ActiveState : public BookingState {
   public:
    bool isBusy() const override { return true; }
};

class OverdueState : public BookingState {
   public:
    bool isBusy() const override { return true; }
};

class ReturnedState : public BookingState {
   public:
    bool isBusy() const override { return false; }
};

class CancelledState : public BookingState {
   public:
    bool isBusy() const override { return false; }
};

}  // namespace

const BookingState& bookingState(BookingStatus status) {
    static const ReservedState reserved;
    static const ActiveState active;
    static const OverdueState overdue;
    static const ReturnedState returned;
    static const CancelledState cancelled;

    switch (status) {
        case BookingStatus::Reserved:
            return reserved;
        case BookingStatus::Active:
            return active;
        case BookingStatus::Overdue:
            return overdue;
        case BookingStatus::Returned:
            return returned;
        case BookingStatus::Cancelled:
            return cancelled;
    }
    throw std::logic_error("Unknown booking status");
}

}  // namespace share
