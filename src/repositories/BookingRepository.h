#pragma once

#include <optional>
#include <vector>

#include "Database.h"
#include "Entities.h"

namespace share {

class BookingRepository {
   public:
    explicit BookingRepository(Database& db) : db_(db) {}
    std::vector<BookingShort> findBookingsByItemId(int itemId);
    int updateAllBookingsOfUser(int userId);

   private:
    Database& db_;
};

}  // namespace share