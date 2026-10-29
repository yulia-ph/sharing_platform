#include "BookingService.h"

namespace share {

std::vector<BookingShort> BookingService::findShortBookingsOfItem(int itemId) {
    if (itemId <= 0) return {};
    return bookings_.findShortBookingsByItemId(itemId);
}

void BookingService::refreshOverdue(UserBooking& booking) {
    if (!bookingState(booking.status).becomesOverdue(today(), booking.endDate)) return;
    bookings_.updateBookingStatus(booking.id, bookingStatusToString(BookingStatus::Overdue));
    booking.status = BookingStatus::Overdue;
}

void BookingService::present(UserBooking& booking, bool borrower) {
    refreshOverdue(booking);

    const BookingState& state = bookingState(booking.status);
    booking.canCancel = state.canCancel();
    if (borrower) {
        booking.canPickup = state.canPickup(today(), booking.startDate, booking.endDate);
        booking.canReturn = state.canReturn();
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

    bookings_.markReturned(bookingId, today());
    return Result::success();
}

Result BookingService::cancel(int userId, int bookingId) {
    Booking booking;
    Result loaded = loadBooking(userId, bookingId, false, booking);
    if (!loaded.ok) return loaded;

    if (!bookingState(booking.status).canCancel()) {
        return Result::error("Booking cannot be cancelled");
    }

    bookings_.updateBookingStatus(bookingId, bookingStatusToString(BookingStatus::Cancelled));
    return Result::success();
}

}  // namespace share
