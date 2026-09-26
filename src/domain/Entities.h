#pragma once

#include <optional>
#include <string>

#include "Enums.h"

namespace share {
    
struct User {
    int id = 0;
    std::string username;
    std::string passwordHash;
};

struct Item {
    int id = 0;
    int ownerId = 0;
    std::string title;
    std::string description;
    std::string createdAt;
    bool isDeleted = false;
};

struct Booking {
    int id = 0;
    int itemId = 0;
    int borrowerId = 0;
    int ownerId = 0;
    std::string startDate;
    std::string endDate;
    BookingStatus status = BookingStatus::Reserved;
    std::optional<std::string> returnedAt;
    std::string createdAt;
};
}