#include "BookingService.h"

namespace share {
namespace {

std::string movedNotice(const std::string& title, const std::string& startDate,
                        const std::string& endDate) {
    return "Sorry! Booking of " + title +
           " has been moved forward due to the overdue of previous booking. New dates: from " +
           startDate + " to " + endDate;
}

std::string cancelledNotice(const std::string& title, const std::string& startDate,
                            const std::string& endDate) {
    return "Sorry! Booking of " + title + " on days from " + startDate + " to " + endDate +
           " has been cancelled due to the overdue of previous booking";
}

bool rangesOverlap(const std::string& startA, const std::string& endA, const std::string& startB,
                   const std::string& endB) {
    return startA <= endB && startB <= endA;
}

}  // namespace

std::vector<BookingShort> BookingService::findShortBookingsOfItem(int itemId) {
    if (itemId <= 0) return {};
    refreshItem(itemId);
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
    if (!borrower) return;

    booking.canPickup = state.canPickup(today(), booking.startDate, booking.endDate);
    booking.canReturn = state.canReturn();
    booking.canPay = booking.billFrozen && !booking.paid && booking.amount > 0 &&
                     booking.status == BookingStatus::Cancelled;
}

void BookingService::refreshListedItems(const std::vector<UserBooking>& bookings) {
    std::vector<int> itemIds;
    for (const UserBooking& booking : bookings) {
        bool seen = false;
        for (int itemId : itemIds) {
            if (itemId == booking.itemId) seen = true;
        }
        if (!seen) itemIds.push_back(booking.itemId);
    }
    for (int itemId : itemIds) refreshItem(itemId);
}

void BookingService::attachNotices(int userId, std::vector<UserBooking>& bookings) {
    for (const BookingNotice& notice : bookings_.takeNotices(userId)) {
        for (UserBooking& booking : bookings) {
            if (booking.id == notice.bookingId) booking.notice = notice.text;
        }
    }
}

void BookingService::refreshItem(int itemId) {
    if (itemId <= 0) return;

    std::vector<BookingShort> bookings = bookings_.findShortBookingsByItemId(itemId);
    for (const BookingShort& booking : bookings) {
        if (booking.status != BookingStatus::Active) continue;
        if (!bookingState(booking.status).becomesOverdue(today(), booking.endDate)) continue;
        bookings_.updateBookingStatus(booking.id, bookingStatusToString(BookingStatus::Overdue));
    }

    bookings = bookings_.findShortBookingsByItemId(itemId);
    bool held = false;
    for (const BookingShort& booking : bookings) {
        if (booking.status == BookingStatus::Overdue) held = true;
    }

    auto item = items_.findItemById(itemId);
    const std::string title = item ? item->title : "item";
    const std::string currentDay = today();
    const std::string nextDay = tomorrow();

    if (held) {
        while (true) {
            bookings = bookings_.findShortBookingsByItemId(itemId);
            const BookingShort* nearest = nullptr;
            for (const BookingShort& booking : bookings) {
                if (booking.status != BookingStatus::Reserved) continue;
                if (booking.startDate > currentDay) continue;
                if (!nearest || booking.startDate < nearest->startDate) nearest = &booking;
            }
            if (!nearest) break;

            BookingShort current = *nearest;
            if (nextDay > current.endDate) {
                bookings_.updateBookingStatus(current.id,
                                              bookingStatusToString(BookingStatus::Cancelled));
                bookings_.setNotice(current.id,
                                    cancelledNotice(title, current.startDate, current.endDate));
                continue;
            }
            if (current.startDate == nextDay) break;

            bookings_.updateBookingDates(current.id, nextDay, current.endDate);
            bookings_.setNotice(current.id, movedNotice(title, nextDay, current.endDate));
            break;
        }
        return;
    }

    for (const BookingShort& booking : bookings) {
        if (booking.status != BookingStatus::Reserved) continue;
        if (!(currentDay > booking.endDate)) continue;
        auto full = bookings_.findBookingById(booking.id);
        if (!full) continue;
        const double amount = bookingCharge(BookingStatus::Reserved, full->startDate, full->endDate,
                                            currentDay, full->pricePerDay);
        bookings_.settleUncollected(booking.id, amount);
    }
}

std::vector<UserBooking> BookingService::findMyBookings(int userId) {
    if (userId <= 0) return {};

    std::vector<UserBooking> bookings = bookings_.findBookingsByBorrowerId(userId);
    refreshListedItems(bookings);
    bookings = bookings_.findBookingsByBorrowerId(userId);
    for (UserBooking& booking : bookings) present(booking, true);
    attachNotices(userId, bookings);
    return bookings;
}

std::vector<UserBooking> BookingService::findIncomingBookings(int userId) {
    if (userId <= 0) return {};

    std::vector<UserBooking> bookings = bookings_.findBookingsByOwnerId(userId);
    refreshListedItems(bookings);
    bookings = bookings_.findBookingsByOwnerId(userId);
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

    refreshItem(booking.itemId);
    loaded = loadBooking(userId, bookingId, true, booking);
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

    refreshItem(booking.itemId);
    loaded = loadBooking(userId, bookingId, false, booking);
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

Result BookingService::createBooking(int itemId, int borrowerId, const std::string& startDate,
                                     const std::string& endDate) {
    if (itemId <= 0) return Result::error("Invalid item id");
    if (borrowerId <= 0) return Result::error("Not logged in");

    if (!isValidDate(startDate) || !isValidDate(endDate) || inclusiveDays(startDate, endDate) == 0 ||
        startDate < today()) {
        return Result::error("Invalid date");
    }

    auto item = items_.findItemById(itemId);
    if (!item || item->isDeleted) return Result::error("Item not found");
    if (item->ownerId == borrowerId) return Result::error("You cannot book your own item");

    refreshItem(itemId);
    for (const BookingShort& booking : bookings_.findShortBookingsByItemId(itemId)) {
        if (!bookingState(booking.status).isBusy()) continue;
        if (rangesOverlap(startDate, endDate, booking.startDate, booking.endDate)) {
            return Result::error("These dates are busy");
        }
    }

    Booking booking;
    booking.itemId = itemId;
    booking.borrowerId = borrowerId;
    booking.ownerId = item->ownerId;
    booking.startDate = startDate;
    booking.endDate = endDate;
    booking.status = BookingStatus::Reserved;
    booking.createdAt = today();

    int id = bookings_.insertBooking(booking);
    if (id <= 0) return Result::error("Database error");
    return Result::success(id);
}

int BookingService::GetBookingsCount() const {
    return bookings_.GetBusyBookingsCount();
}

}  // namespace share
