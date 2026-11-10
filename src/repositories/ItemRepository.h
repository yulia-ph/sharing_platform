#pragma once

#include <optional>
#include <vector>

#include "Database.h"
#include "Entities.h"

namespace share {

class ItemRepository {
   public:
    explicit ItemRepository(Database& db) : db_(db) {}
    std::vector<ItemShort> findItemByUserId(int userId);
    std::optional<Item> findItemById(int itemId);
    bool editItem(int itemId, const Item& newItem);
    bool deleteItem(int itemId);
    std::vector<std::string> findCities();
    std::vector<ItemShort> findItemsByCityCat(const std::string& city, const std::string& category);
    std::vector<ItemShort> findItemsByTitleCat(const std::string& city, const std::string& title);

   private:
    Database& db_;
};

}  // namespace share