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
    void updateBookingStatus(int bookingId, const std::string& status);
    std::vector<BookingShort> findShortBookingsByBorrowerId(int borrowerId);
    std::vector<BookingShort> findShortBookingsByOwnerId(int ownerId);
    std::optional<Booking> findBookingByBookingdId(int bookingId);

   private:
    Database& db_;
};

}  // namespace share