#pragma once

#include <vector>

#include "BookingCharge.h"
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
    Result pay(int userId, int bookingId);
    std::vector<std::string> BookingService::popNotifications();
    Result createBooking(int itemId, int borrowerId, int ownerId, std::string startDate,
                         std::string endDate);

   private:
    Result loadBooking(int userId, int bookingId, bool borrowerOnly, Booking& booking);
    void settleUncollected(UserBooking& booking);
    void refreshOverdue(UserBooking& booking);
    void present(UserBooking& booking, bool borrower);

    BookingRepository& bookings_;
    std::vector<std::string> notifications_;
};

}  // namespace share
