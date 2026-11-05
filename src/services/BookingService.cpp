#include "BookingService.h"

namespace share {

std::vector<BookingShort> BookingService::findShortBookingsOfItem(int itemId) {
    if (itemId <= 0) return {};
    return bookings_.findShortBookingsByItemId(itemId);
}

void BookingService::settleUncollected(UserBooking& booking) {
    if (booking.billFrozen || booking.status != BookingStatus::Reserved) return;
    const std::string currentDay = today();
    if (!(currentDay > booking.endDate)) return;

    const double amount = bookingCharge(BookingStatus::Reserved, booking.startDate, booking.endDate,
                                        currentDay, booking.pricePerDay);
    bookings_.settleUncollected(booking.id, amount);
    booking.status = BookingStatus::Cancelled;
    booking.amount = amount;
    booking.billFrozen = true;
    booking.paid = false;
}

void BookingService::refreshOverdue(UserBooking& booking) {
    if (booking.status != BookingStatus::Active) return;
    if (!bookingState(booking.status).becomesOverdue(today(), booking.endDate)) return;
    bookings_.updateBookingStatus(booking.id, bookingStatusToString(BookingStatus::Overdue));
    booking.status = BookingStatus::Overdue;
}

void BookingService::present(UserBooking& booking, bool borrower) {
    settleUncollected(booking);
    refreshOverdue(booking);
    if (!booking.billFrozen) {
        booking.amount = bookingCharge(booking.status, booking.startDate, booking.endDate, today(),
                                booking.pricePerDay);
    }

    const BookingState& state = bookingState(booking.status);
    booking.canCancel = state.canCancel();
    booking.canPickup = false;
    booking.canReturn = false;
    booking.canPay = false;
    if (borrower) {
        booking.canPickup = state.canPickup(today(), booking.startDate, booking.endDate);
        booking.canReturn = state.canReturn();
        booking.canPay = booking.billFrozen && !booking.paid && booking.amount > 0 &&
                         booking.status == BookingStatus::Cancelled;
    }
}

std::vector<UserBooking> BookingService::findMyBookings(int userId) {
    if (userId <= 0) return {};

    std::vector<UserBooking> bookings = bookings_.findBookingsByBorrowerId(userId);
    for (UserBooking& booking : bookings) present(booking, true);
    return bookings;
}

std::vector<UserBooking> BookingService::findIncomingBookings(int userId) {
    if (userId <= 0) return {};

    std::vector<UserBooking> bookings = bookings_.findBookingsByOwnerId(userId);
    for (UserBooking& booking : bookings) present(booking, false);
    return bookings;
}

Result BookingService::loadBooking(int userId, int bookingId, bool borrowerOnly, Booking& booking) {
    if (bookingId <= 0) return Result::error("Invalid booking id");

    auto found = bookings_.findBookingById(bookingId);
    if (!found) return Result::error("Booking not found");

    if (found->ownerId != userId && found->borrowerId != userId) {
        return Result::error("You don't have access to this booking");
    }
    if (borrowerOnly && found->borrowerId != userId) {
        return Result::error("You don't have access to this booking");
    }

    booking = *found;
    return Result::success();
}

Result BookingService::pickup(int userId, int bookingId) {
    Booking booking;
    Result loaded = loadBooking(userId, bookingId, true, booking);
    if (!loaded.ok) return loaded;

    if (!bookingState(booking.status).canPickup(today(), booking.startDate, booking.endDate)) {
        return Result::error("Booking cannot be picked up");
    }

    bookings_.updateBookingStatus(bookingId, bookingStatusToString(BookingStatus::Active));
    return Result::success();
}

Result BookingService::returnBooking(int userId, int bookingId) {
    Booking booking;
    Result loaded = loadBooking(userId, bookingId, true, booking);
    if (!loaded.ok) return loaded;

    if (!bookingState(booking.status).canReturn()) {
        return Result::error("Booking cannot be returned");
    }

    const std::string currentDay = today();
    BookingStatus rate = booking.status;
    if (rate == BookingStatus::Active && currentDay > booking.endDate) {
        rate = BookingStatus::Overdue;
    }
    const double amount =
        bookingCharge(rate, booking.startDate, booking.endDate, currentDay, booking.pricePerDay);
    bookings_.markReturned(bookingId, currentDay, amount);
    return Result::success();
}

Result BookingService::cancel(int userId, int bookingId) {
    Booking booking;
    Result loaded = loadBooking(userId, bookingId, false, booking);
    if (!loaded.ok) return loaded;

    if (booking.status == BookingStatus::Reserved && today() > booking.endDate) {
        const double amount = bookingCharge(BookingStatus::Reserved, booking.startDate,
                                            booking.endDate, today(), booking.pricePerDay);
        bookings_.settleUncollected(booking.id, amount);
        return Result::error("Booking cannot be cancelled");
    }

    if (!bookingState(booking.status).canCancel()) {
        return Result::error("Booking cannot be cancelled");
    }

    bookings_.updateBookingStatus(bookingId, bookingStatusToString(BookingStatus::Cancelled));
    return Result::success();
}

Result BookingService::pay(int userId, int bookingId) {
    Booking booking;
    Result loaded = loadBooking(userId, bookingId, true, booking);
    if (!loaded.ok) return loaded;

    if (booking.status != BookingStatus::Cancelled || !booking.amount || *booking.amount <= 0 ||
        booking.paid) {
        return Result::error("Nothing to pay");
    }

    if (!bookings_.markPaid(bookingId)) return Result::error("Nothing to pay");
    return Result::success();
}

}  // namespace share
