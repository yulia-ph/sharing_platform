#include "BookingState.h"

#include <stdexcept>

namespace share {
namespace {

class ReservedState : public BookingState {
   public:
    bool isBusy() const override { return true; }

    bool canPickup(const std::string& today, const std::string& startDate,
                   const std::string& endDate) const override {
        return today >= startDate && today <= endDate;
    }

    bool canReturn() const override { return false; }
    bool canCancel() const override { return true; }

    bool becomesOverdue(const std::string& today, const std::string& endDate) const override {
        return today > endDate;
    }
};

class ActiveState : public BookingState {
   public:
    bool isBusy() const override { return true; }

    bool canPickup(const std::string&, const std::string&, const std::string&) const override {
        return false;
    }

    bool canReturn() const override { return true; }
    bool canCancel() const override { return false; }

    bool becomesOverdue(const std::string& today, const std::string& endDate) const override {
        return today > endDate;
    }
};

class OverdueState : public BookingState {
   public:
    bool isBusy() const override { return true; }

    bool canPickup(const std::string&, const std::string&, const std::string&) const override {
        return false;
    }

    bool canReturn() const override { return true; }
    bool canCancel() const override { return false; }

    bool becomesOverdue(const std::string&, const std::string&) const override { return false; }
};

class ReturnedState : public BookingState {
   public:
    bool isBusy() const override { return false; }

    bool canPickup(const std::string&, const std::string&, const std::string&) const override {
        return false;
    }

    bool canReturn() const override { return false; }
    bool canCancel() const override { return false; }

    bool becomesOverdue(const std::string&, const std::string&) const override { return false; }
};

class CancelledState : public BookingState {
   public:
    bool isBusy() const override { return false; }

    bool canPickup(const std::string&, const std::string&, const std::string&) const override {
        return false;
    }

    bool canReturn() const override { return false; }
    bool canCancel() const override { return false; }

    bool becomesOverdue(const std::string&, const std::string&) const override { return false; }
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
