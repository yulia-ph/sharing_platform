#pragma once

#include <cctype>
#include <optional>
#include <vector>

#include "BookingRepository.h"
#include "Database.h"
#include "DateUtils.h"
#include "Entities.h"
#include "Enums.h"
#include "ItemRepository.h"
#include "Result.h"

namespace share {

class ItemService {
   public:
    explicit ItemService(ItemRepository& items, BookingRepository& bookings, Database& db)
        : items_(items), bookings_(bookings), db_(db) {}

    std::vector<ItemShort> findUserItems(int userId);
    std::optional<Item> findItemInfo(int itemId);
    std::vector<std::string> getCategories();
    std::string availability(int itemId);
    Result addItem(int ownerId, const std::string& title, const std::string& description,
                   const std::string& category, const std::string& city);
    Result editItem(int userId, int itemId, const std::string& title,
                    const std::string& description, const std::string& category,
                    const std::string& city);
    Result deleteItem(int userId, int itemId);

   private:
    ItemRepository& items_;
    BookingRepository& bookings_;
    Database& db_;
};
}  // namespace share