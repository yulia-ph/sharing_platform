#pragma once

#include <cctype>
#include <optional>
#include <vector>

#include "BookingRepository.h"
#include "Entities.h"
#include "ItemRepository.h"
#include "Result.h"

namespace share {

class ItemService {
   public:
    explicit ItemService(ItemRepository& items, BookingRepository& bookings)
        : items_(items), bookings_(bookings) {}

    std::vector<ItemShort> findUserItems(int userId);
    std::optional<Item> findItemInfo(int itemId);
    std::vector<BookingShort> findBookingsOfItem(int itemId);
    std::vector<std::string> getCategories();
    Result addItem(int ownerId, const std::string& title, const std::string& description,
                   const std::string& category);
    Result editItem(int itemId, const std::string& title, const std::string& description,
                    const std::string& category);
    Result deleteItem(int itemId);

   private:
    ItemRepository& items_;
    BookingRepository& bookings_;
};
}  // namespace share