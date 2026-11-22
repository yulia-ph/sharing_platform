#pragma once

#include <optional>
#include <string>
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
    std::vector<std::string> findCities();

    std::vector<ItemShort> findItems(int userId, const std::string& city,
                                     const std::string& category, const std::string& title,
                                     int limit, int offset);

    int countItems(int userId, const std::string& city, const std::string& category,
                   const std::string& title);

    int GetNotDeletedItemsCount();

    void populateTitleVocab();

   private:
    Database& db_;
    std::vector<ItemShort> searchByLike(int userId, const std::string& city,
                                        const std::string& category, const std::string& title,
                                        int limit, int offset);

    std::vector<ItemShort> searchByFts(int userId, const std::string& city,
                                       const std::string& category, const std::string& title,
                                       int limit, int offset);

    void addWordsToVocab(const std::string& title);
};

}  // namespace share