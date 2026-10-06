#include "Enums.h"

namespace share {

std::string bookingStatusToString(BookingStatus status) {
    switch (status) {
        case BookingStatus::Reserved:  return "reserved";
        case BookingStatus::Active:    return "active";
        case BookingStatus::Returned:  return "returned";
        case BookingStatus::Cancelled: return "cancelled";
    }
}

BookingStatus bookingStatusFromString(const std::string& s) {
    if (s == "reserved")  return BookingStatus::Reserved;
    if (s == "active")    return BookingStatus::Active;
    if (s == "returned")  return BookingStatus::Returned;
    if (s == "cancelled") return BookingStatus::Cancelled;
    throw std::runtime_error("Неизвестный статус: " + s);
}

std::string itemCategoryToString(ItemCategory category){
    switch (category) {
        case ItemCategory::Electronics:  return "electronics";
        case ItemCategory::Toys:         return "toys";
        case ItemCategory::ForKitchen:   return "for kitchen";
        case ItemCategory::Sports:       return "sports";
        case ItemCategory::Books:        return "books";
        case ItemCategory::Leisure:      return "leisure";
        case ItemCategory::Tools:        return "tools";
        case ItemCategory::Services:     return "services";
        case ItemCategory::Other:        return "other";
    }
};

ItemCategory itemCategoryFromString(const std::string& s){
    if (s == "electronics")  return ItemCategory::Electronics;
    if (s == "toys")         return ItemCategory::Toys;
    if (s == "for kitchen")  return ItemCategory::ForKitchen;
    if (s == "sports")       return ItemCategory::Sports;
    if (s == "books")        return ItemCategory::Books;
    if (s == "leisure")      return ItemCategory::Leisure;
    if (s == "tools")        return ItemCategory::Tools;
    if (s == "services")     return ItemCategory::Services;
    if (s == "other")        return ItemCategory::Other;
    throw std::runtime_error("Неизвестная категория: " + s);
};

}