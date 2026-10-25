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
    void updateBookingStatus(int bookingId, BookingStatus newStatus);

   private:
    Database& db_;
};

}  // namespace share