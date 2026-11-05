#include "BookingRepository.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>

#include "Enums.h"
#include "sqliteColumn.h"

namespace share {
namespace {

std::vector<UserBooking> readUserBookings(sqlite3_stmt* stmt, const std::string& context) {
    std::vector<UserBooking> bookings;

    while (true) {
        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_DONE) break;
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(sqlite3_db_handle(stmt));
            sqlite3_finalize(stmt);
            throw std::runtime_error(context + ": " + err);
        }

        UserBooking booking;
        booking.id = column<int>(stmt, 0);
        booking.title = column<std::string>(stmt, 1);
        booking.city = column<std::string>(stmt, 2);
        booking.username = column<std::string>(stmt, 3);
        booking.startDate = column<std::string>(stmt, 4);
        booking.endDate = column<std::string>(stmt, 5);
        std::string status = column<std::string>(stmt, 6);
        booking.status = bookingStatusFromString(status.empty() ? "reserved" : status);
        booking.pricePerDay = column<double>(stmt, 7);
        if (sqlite3_column_type(stmt, 8) == SQLITE_NULL) {
            booking.billFrozen = false;
            booking.amount = 0;
        } else {
            booking.billFrozen = true;
            booking.amount = column<double>(stmt, 8);
        }
        booking.paid = column<int>(stmt, 9) != 0;
        bookings.push_back(booking);
    }

    sqlite3_finalize(stmt);
    return bookings;
}

}  // namespace

std::vector<BookingShort> BookingRepository::findShortBookingsByItemId(int itemId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT bookings.id, users.username, bookings.start_date, bookings.end_date, "
        "bookings.status "
        "FROM bookings JOIN users ON users.id = bookings.borrower_id "
        "WHERE bookings.item_id = ? ORDER BY bookings.start_date";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::findShortBookingsByItemId: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, itemId);
    std::vector<BookingShort> bookings;

    while (true) {
        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_DONE) break;
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("BookingRepository::findShortBookingsByItemId: " + err);
        }

        BookingShort booking;
        booking.id = column<int>(stmt, 0);
        booking.username = column<std::string>(stmt, 1);
        booking.startDate = column<std::string>(stmt, 2);
        booking.endDate = column<std::string>(stmt, 3);
        std::string status = column<std::string>(stmt, 4);
        booking.status = bookingStatusFromString(status.empty() ? "reserved" : status);
        bookings.push_back(booking);
    }

    sqlite3_finalize(stmt);
    return bookings;
}

std::vector<UserBooking> BookingRepository::findBookingsByBorrowerId(int borrowerId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT bookings.id, items.title, items.city, owners.username, "
        "bookings.start_date, bookings.end_date, bookings.status, items.price_per_day, "
        "bookings.amount, bookings.paid "
        "FROM bookings "
        "JOIN items ON items.id = bookings.item_id "
        "JOIN users AS owners ON owners.id = items.owner_id "
        "WHERE bookings.borrower_id = ? ORDER BY bookings.start_date";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::findBookingsByBorrowerId: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, borrowerId);
    return readUserBookings(stmt, "BookingRepository::findBookingsByBorrowerId");
}

std::vector<UserBooking> BookingRepository::findBookingsByOwnerId(int ownerId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT bookings.id, items.title, items.city, borrowers.username, "
        "bookings.start_date, bookings.end_date, bookings.status, items.price_per_day, "
        "bookings.amount, bookings.paid "
        "FROM bookings "
        "JOIN items ON items.id = bookings.item_id "
        "JOIN users AS borrowers ON borrowers.id = bookings.borrower_id "
        "WHERE items.owner_id = ? ORDER BY bookings.start_date";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::findBookingsByOwnerId: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, ownerId);
    return readUserBookings(stmt, "BookingRepository::findBookingsByOwnerId");
}

std::optional<Booking> BookingRepository::findBookingById(int bookingId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT bookings.id, bookings.item_id, bookings.borrower_id, items.owner_id, "
        "bookings.start_date, bookings.end_date, bookings.status, bookings.returned_at, "
        "bookings.created_at, items.price_per_day, bookings.amount, bookings.paid "
        "FROM bookings JOIN items ON items.id = bookings.item_id "
        "WHERE bookings.id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::findBookingById: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, bookingId);
    int stepResult = sqlite3_step(stmt);
    std::optional<Booking> result;

    if (stepResult == SQLITE_ROW) {
        Booking booking;
        booking.id = column<int>(stmt, 0);
        booking.itemId = column<int>(stmt, 1);
        booking.borrowerId = column<int>(stmt, 2);
        booking.ownerId = column<int>(stmt, 3);
        booking.startDate = column<std::string>(stmt, 4);
        booking.endDate = column<std::string>(stmt, 5);
        std::string status = column<std::string>(stmt, 6);
        booking.status = bookingStatusFromString(status.empty() ? "reserved" : status);
        booking.returnedAt = column<std::optional<std::string>>(stmt, 7);
        booking.createdAt = column<std::string>(stmt, 8);
        booking.pricePerDay = column<double>(stmt, 9);
        booking.amount=column<std::optional<double>>(stmt, 10);
        booking.paid = column<int>(stmt, 11) != 0;
        result = booking;
    }

    if (stepResult != SQLITE_ROW && stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("BookingRepository::findBookingById: " + err);
    }

    sqlite3_finalize(stmt);
    return result;
}

void BookingRepository::updateBookingStatus(int bookingId, const std::string& status) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE bookings SET status = ? WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::updateBookingStatus: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, bookingId);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("BookingRepository::updateBookingStatus: " + err);
    }

    sqlite3_finalize(stmt);
}

void BookingRepository::settleUncollected(int bookingId, double amount) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "UPDATE bookings SET status = 'cancelled', amount = ?, paid = 0 "
        "WHERE id = ? AND status = 'reserved'";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::settleUncollected: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_double(stmt, 1, amount);
    sqlite3_bind_int(stmt, 2, bookingId);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("BookingRepository::settleUncollected: " + err);
    }

    sqlite3_finalize(stmt);
}

void BookingRepository::markReturned(int bookingId, const std::string& returnedAt, double amount) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "UPDATE bookings SET status = 'returned', returned_at = ?, amount = ?, paid = 1 "
        "WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::markReturned: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_text(stmt, 1, returnedAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, amount);
    sqlite3_bind_int(stmt, 3, bookingId);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("BookingRepository::markReturned: " + err);
    }

    sqlite3_finalize(stmt);
}

bool BookingRepository::markPaid(int bookingId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "UPDATE bookings SET paid = 1 "
        "WHERE id = ? AND paid = 0 AND amount IS NOT NULL";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("BookingRepository::markPaid: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, bookingId);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("BookingRepository::markPaid: " + err);
    }

    int changed = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changed > 0;
}

}  // namespace share