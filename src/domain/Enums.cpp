#include "Enums.h"

namespace share {

std::string toString(BookingStatus status) {
    switch (status) {
        case BookingStatus::Reserved:  return "reserved";
        case BookingStatus::Active:    return "active";
        case BookingStatus::Returned:  return "returned";
        case BookingStatus::Cancelled: return "cancelled";
    }
    return "reserved"; // на всякий случай
}

BookingStatus bookingStatusFromString(const std::string& s) {
    if (s == "reserved")  return BookingStatus::Reserved;
    if (s == "active")    return BookingStatus::Active;
    if (s == "returned")  return BookingStatus::Returned;
    if (s == "cancelled") return BookingStatus::Cancelled;
    throw std::runtime_error("Неизвестный статус: " + s);
}
}