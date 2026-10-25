#include "ItemRepository.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>

#include "Enums.h"
#include "sqliteColumn.h"

namespace share {

std::vector<ItemShort> ItemRepository::findItemByUserId(int userId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT id, title, city FROM items WHERE owner_id = ? AND is_deleted = 0 ORDER BY id";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::findItemByUserId: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, userId);
    std::vector<ItemShort> items;

    while (true) {
        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_DONE) break;
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("ItemRepository::findItemByUserId: " + err);
        }

        ItemShort item;
        item.id = column<int>(stmt, 0);
        item.title = column<std::string>(stmt, 1);
        item.city = column<std::string>(stmt, 2);
        items.push_back(item);
    }

    sqlite3_finalize(stmt);
    return items;
}

std::optional<Item> ItemRepository::findItemById(int itemId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT id, owner_id, title, description, created_at, is_deleted, category, city "
        "FROM items WHERE id = ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::findItemById: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, itemId);
    int stepResult = sqlite3_step(stmt);
    std::optional<Item> result;

    if (stepResult == SQLITE_ROW) {
        Item item;
        item.id = column<int>(stmt, 0);
        item.ownerId = column<int>(stmt, 1);
        item.title = column<std::string>(stmt, 2);
        item.description = column<std::string>(stmt, 3);
        item.createdAt = column<std::string>(stmt, 4);
        item.isDeleted = column<int>(stmt, 5) != 0;
        std::string category = column<std::string>(stmt, 6);
        item.category = itemCategoryFromString(category.empty() ? "other" : category);
        item.city = column<std::string>(stmt, 7);
        result = item;
    }

    if (stepResult != SQLITE_ROW && stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::findItemById: " + err);
    }

    sqlite3_finalize(stmt);
    return result;
}

int ItemRepository::insertItem(const Item& item) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "INSERT INTO items (owner_id, title, description, category, created_at, city) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::insertItem: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    std::string category = itemCategoryToString(item.category);
    sqlite3_bind_int(stmt, 1, item.ownerId);
    sqlite3_bind_text(stmt, 2, item.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, item.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, item.createdAt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, item.city.c_str(), -1, SQLITE_TRANSIENT);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::insertItem: " + err);
    }
    sqlite3_finalize(stmt);
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

bool ItemRepository::editItem(int itemId, const Item& newItem) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "UPDATE items SET title = ?, description = ?, category = ?, city = ? "
        "WHERE id = ? AND is_deleted = 0";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::editItem: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    std::string category = itemCategoryToString(newItem.category);
    sqlite3_bind_text(stmt, 1, newItem.title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, newItem.description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, newItem.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, itemId);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::editItem: " + err);
    }

    int changed = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changed > 0;
}

bool ItemRepository::deleteItem(int itemId) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "UPDATE items SET is_deleted = 1 WHERE id = ? AND is_deleted = 0";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::deleteItem: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    sqlite3_bind_int(stmt, 1, itemId);
    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::deleteItem: " + err);
    }

    int changed = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);
    return changed > 0;
}

}  // namespace share
