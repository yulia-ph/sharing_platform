#pragma once

#include <optional>
#include <vector>

#include "Database.h"
#include "Entities.h"

namespace share {

class BookingRepository {
   public:
    explicit BookingRepository(Database& db) : db_(db) {}
    std::vector<BookingShort> findShortBookingsByItemId(int itemId);
    std::vector<UserBooking> findBookingsByBorrowerId(int borrowerId);
    std::vector<UserBooking> findBookingsByOwnerId(int ownerId);
    std::optional<Booking> findBookingById(int bookingId);
    void updateBookingStatus(int bookingId, const std::string& status);
    void settleUncollected(int bookingId, double amount);
    void markReturned(int bookingId, const std::string& returnedAt, double amount);
    bool markPaid(int bookingId);

   private:
    Database& db_;
};

}  // namespace share