#include "BookingRepository.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>

#include "Enums.h"
#include "sqliteColumn.h"

namespace share {

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
        booking.borrowerUsername = column<std::string>(stmt, 1);
        booking.startDate = column<std::string>(stmt, 2);
        booking.endDate = column<std::string>(stmt, 3);
        std::string status = column<std::string>(stmt, 4);
        booking.status = bookingStatusFromString(status.empty() ? "reserved" : status);
        bookings.push_back(booking);
    }

    sqlite3_finalize(stmt);
    return bookings;
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

}  // namespace share