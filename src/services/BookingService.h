#pragma once

#include <vector>

#include "BookingRepository.h"
#include "BookingState.h"
#include "DateUtils.h"
#include "Entities.h"
#include "Enums.h"
#include "Result.h"

namespace share {

class BookingService {
   public:
    explicit BookingService(BookingRepository& bookings) : bookings_(bookings) {}

    std::vector<BookingShort> findShortBookingsOfItem(int itemId);
    std::vector<UserBooking> findMyBookings(int userId);
    std::vector<UserBooking> findIncomingBookings(int userId);
    Result pickup(int userId, int bookingId);
    Result returnBooking(int userId, int bookingId);
    Result cancel(int userId, int bookingId);

   private:
    Result loadBooking(int userId, int bookingId, bool borrowerOnly, Booking& booking);
    void refreshOverdue(UserBooking& booking);
    void present(UserBooking& booking, bool borrower);

    BookingRepository& bookings_;
};

}  // namespace share
