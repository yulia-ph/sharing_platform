#pragma once

#include <string>

namespace share {

enum class BookingStatus {
    Reserved,
    Active,
    Returned,
    Cancelled,
};

std::string bookingStatusToString(BookingStatus status);

BookingStatus bookingStatusFromString(const std::string& s);

enum class ItemCategory {
    Electronics,
    Toys,
    ForKitchen,
    Sports,
    Books,
    Leisure,
    Tools,
    Services,
    Other,
};

std::string itemCategoryToString(ItemCategory category);

ItemCategory itemCategoryFromString(const std::string& s);


}