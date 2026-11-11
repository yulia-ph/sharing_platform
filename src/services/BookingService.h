#pragma once

#include <vector>

#include "BookingCharge.h"
#include "BookingRepository.h"
#include "BookingState.h"
#include "DateUtils.h"
#include "Entities.h"
#include "Enums.h"
#include "ItemRepository.h"
#include "Result.h"

namespace share {

class BookingService {
   public:
    explicit BookingService(BookingRepository& bookings, ItemRepository& items)
        : bookings_(bookings), items_(items) {}

    std::vector<BookingShort> findShortBookingsOfItem(int itemId);
    std::vector<UserBooking> findMyBookings(int userId);
    std::vector<UserBooking> findIncomingBookings(int userId);
    Result pickup(int userId, int bookingId);
    Result returnBooking(int userId, int bookingId);
    Result cancel(int userId, int bookingId);
    Result pay(int userId, int bookingId);
    Result createBooking(int itemId, int borrowerId, const std::string& startDate,
                         const std::string& endDate);
    void refreshItem(int itemId);

   private:
    Result loadBooking(int userId, int bookingId, bool borrowerOnly, Booking& booking);
    void settleUncollected(UserBooking& booking);
    void refreshOverdue(UserBooking& booking);
    void present(UserBooking& booking, bool borrower);
    void refreshListedItems(const std::vector<UserBooking>& bookings);
    void attachNotices(int userId, std::vector<UserBooking>& bookings);

    BookingRepository& bookings_;
    ItemRepository& items_;
};

}  // namespace share
