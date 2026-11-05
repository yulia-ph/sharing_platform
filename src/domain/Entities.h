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
    ItemCategory category = ItemCategory::Other;
    std::string city;
    double pricePerDay = 0;
};

struct ItemShort {
    int id = 0;
    std::string title;
    std::string city;
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
    double pricePerDay = 0;
    std::optional<double> amount;
    bool paid = false;
};

struct BookingShort {
    int id = 0;
    std::string username;
    std::string startDate;
    std::string endDate;
    BookingStatus status = BookingStatus::Reserved;
};

struct UserBooking {
    int id = 0;
    std::string title;
    std::string city;
    std::string username;
    std::string startDate;
    std::string endDate;
    BookingStatus status = BookingStatus::Reserved;
    bool canPickup = false;
    bool canReturn = false;
    bool canCancel = false;
    bool canPay = false;
    double pricePerDay = 0;
    double amount = 0;
    bool billFrozen = false;
    bool paid = false;
};

}  // namespace share