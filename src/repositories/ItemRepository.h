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
    int insertItem(const Item& item);
    bool editItem(int itemId, const Item& newItem);
    bool deleteItem(int itemId);

   private:
    Database& db_;
};

}  // namespace share