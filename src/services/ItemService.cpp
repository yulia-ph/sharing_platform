#include "ItemService.h"

#include <cctype>

#include "BookingState.h"

namespace share {

static std::string trim(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start]))) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1]))) {
        --end;
    }
    return value.substr(start, end - start);
}

static std::string validateCity(const std::string& city) {
    if (trim(city).empty()) return "City cannot be empty";
    return "";
}

std::vector<ItemShort> ItemService::findUserItems(int userId) {
    if (userId <= 0) {
        return {};
    }

    return items_.findItemByUserId(userId);
}

std::optional<Item> ItemService::findItemInfo(int itemId) {
    if (itemId <= 0) {
        return std::nullopt;
    }

    return items_.findItemById(itemId);
}

std::vector<std::string> ItemService::getCategories() {
    std::vector<std::string> names;
    for (ItemCategory category : allCategories()) {
        names.push_back(itemCategoryToString(category));
    }
    return names;
}

std::string ItemService::availability(int itemId) {
    for (const BookingShort& booking : bookings_.findShortBookingsByItemId(itemId)) {
        if (bookingState(booking.status).isBusy()) return "busy";
    }
    return "free";
}

Result ItemService::addItem(int ownerId, const std::string& title, const std::string& description,
                            const std::string& category, const std::string& city) {
    if (ownerId <= 0) {
        return Result::error("Invalid owner id");
    }

    ItemCategory cat;

    try {
        cat = itemCategoryFromString(category);
    } catch (...) {
        return Result::error("Invalid category");
    }

    if (title.empty()) {
        return Result::error("Title cannot be empty");
    }
    if (title.size() > 100) {
        return Result::error("Title is too long (max 100 characters)");
    }
    std::string cityError = validateCity(city);
    if (!cityError.empty()) return Result::error(cityError);

    Item item;
    item.ownerId = ownerId;
    item.title = title;
    item.description = description;
    item.createdAt = today();
    item.category = cat;
    item.city = trim(city);

    item.id = items_.insertItem(item);

    if (item.id == -1) {
        return Result::error("Database error");
    }

    return Result::success(item.id);
}

Result ItemService::editItem(int userId, int itemId, const std::string& title,
                             const std::string& description, const std::string& category,
                             const std::string& city) {
    if (itemId <= 0) {
        return Result::error("Invalid item id");
    }

    auto item = items_.findItemById(itemId);
    if (!item || item->isDeleted) {
        return Result::error("Item not found");
    }
    if (item->ownerId != userId) {
        return Result::error("You don't have access to this item");
    }

    ItemCategory cat;

    try {
        cat = itemCategoryFromString(category);
    } catch (...) {
        return Result::error("Invalid category");
    }

    if (title.empty()) {
        return Result::error("Title cannot be empty");
    }
    if (title.size() > 100) {
        return Result::error("Title is too long (max 100 characters)");
    }
    std::string cityError = validateCity(city);
    if (!cityError.empty()) return Result::error(cityError);

    item->title = title;
    item->description = description;
    item->category = cat;
    item->city = trim(city);

    if (!items_.editItem(itemId, *item)) {
        return Result::error("Failed to update item");
    }

    return Result::success();
}

Result ItemService::deleteItem(int userId, int itemId) {
    if (itemId <= 0) {
        return Result::error("Invalid item id");
    }

    auto item = items_.findItemById(itemId);
    if (!item || item->isDeleted) {
        return Result::error("Item not found");
    }

    if (item->ownerId != userId) {
        return Result::error("You don't have access to this item");
    }

    std::vector<BookingShort> allBookings = bookings_.findShortBookingsByItemId(itemId);

    try {
        db_.beginTransaction();

        if (!items_.deleteItem(itemId)) {
            throw std::runtime_error("Failed delete item");
        }

        for (BookingShort booking : allBookings) {
            if (booking.status == BookingStatus::Reserved ||
                booking.status == BookingStatus::Active) {
                bookings_.updateBookingStatus(booking.id, BookingStatus::Cancelled);
            }
        }

        db_.commit();
    } catch (const std::exception& e) {
        db_.rollback();
        return Result::error(std::string("Failed to delete item: ") + e.what());
    }

    return Result::success();
}

}  // namespace share