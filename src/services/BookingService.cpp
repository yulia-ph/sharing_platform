#include "BookingService.h"

namespace share {

std::vector<BookingShort> BookingService::findShortBookingsOfItem(int itemId) {
    if (itemId <= 0) {
        return {};
    }

    return bookings_.findShortBookingsByItemId(itemId);
}

}  // namespace share
