#include "ItemService.h"

namespace share {

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

std::vector<BookingShort> ItemService::findBookingsOfItem(int itemId) {
    if (itemId <= 0) {
        return {};
    }

    return bookings_.findShortBookingsByItemId(itemId);
}

std::vector<std::string> ItemService::getCategories() {
    std::vector<std::string> result;

    for (auto category : allCategories()) {
        result.push_back(itemCategoryToString(category));
    }

    return result;
}

Result ItemService::addItem(int ownerId, const std::string& title, const std::string& description,
                            const std::string& category) {
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

    Item item;
    item.ownerId = ownerId;
    item.title = title;
    item.description = description;
    item.createdAt = today();
    item.category = cat;

    item.id = items_.insertItem(item);

    if (item.id == -1) {
        return Result::error("Database error");
    }

    return Result::success();
}

Result ItemService::editItem(int userId, int itemId, const std::string& title,
                             const std::string& description, const std::string& category) {
    if (itemId <= 0) {
        return Result::error("Invalid item id");
    }

    auto item = items_.findItemById(itemId);
    if (!item) {
        return Result::error("This item does not exist");
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

    item->title = title;
    item->description = description;
    item->category = cat;

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
    if (!item) {
        return Result::error("This item does not exist");
    }

    if (item->isDeleted == true) {
        return Result::error("Item already deleted");
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