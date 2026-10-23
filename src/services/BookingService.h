#include <optional>
#include <vector>

#include "BookingRepository.h"
#include "Database.h"
#include "Entities.h"
#include "Enums.h"
#include "Result.h"

namespace share {

class BookingService {
   public:
    explicit BookingService(BookingRepository& bookings, Database& db_)
        : bookings_(bookings), db_(db_) {}

    std::vector<BookingShort> findShortBookingsOfItem(int itemId);

   private:
    BookingRepository& bookings_;
    Database& db_;
};

}  // namespace share