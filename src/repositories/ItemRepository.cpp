#include "ItemRepository.h"

#include <sqlite3.h>

#include <cctype>
#include <set>
#include <stdexcept>
#include <string>

#include "Enums.h"
#include "sqliteColumn.h"

namespace share {
namespace {

const char* itemSearchWhere =
    "FROM items "
    "WHERE is_deleted = 0 AND owner_id != ? "
    "AND (? = '' OR lower(city) = lower(?)) "
    "AND (? = '' OR category = ?) "
    "AND (? = '' OR lower(title) LIKE ? ESCAPE '\\') ";

std::string titleLikePattern(const std::string& title) {
    std::string pattern;
    for (unsigned char symbol : title) {
        char c = static_cast<char>(symbol);
        if (c == '\\' || c == '%' || c == '_') pattern.push_back('\\');
        pattern.push_back(c);
    }
    return "%" + pattern + "%";
}

void bindItemSearch(sqlite3_stmt* stmt, int userId, const std::string& city,
                    const std::string& category, const std::string& title) {
    const std::string pattern = titleLikePattern(title);
    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, pattern.c_str(), -1, SQLITE_TRANSIENT);
}

std::string quoteForFts(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    out += '"';
    for (char c : s) {
        if (c == '"') out += '"';
        out += c;
    }
    out += '"';
    return out;
}

std::string correctTitle(Database& db, const std::string& title) {
    std::string corrected;
    std::string currentWord;

    for (size_t i = 0; i <= title.size(); ++i) {
        if (i == title.size() || !std::isalpha(static_cast<unsigned char>(title[i]))) {
            if (!currentWord.empty()) {
                sqlite3_stmt* stmt = nullptr;
                const char* sql = "SELECT word FROM title_vocab WHERE word MATCH ? AND top=1";
                if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, currentWord.c_str(), -1, SQLITE_TRANSIENT);
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        corrected += column<std::string>(stmt, 0);
                    } else {
                        corrected += currentWord;
                    }
                    sqlite3_finalize(stmt);
                } else {
                    corrected += currentWord;
                }

                if (i < title.size()) corrected += title[i];
                currentWord.clear();
            } else if (i < title.size()) {
                corrected += title[i];
            }
        } else {
            currentWord.push_back(title[i]);
        }
    }
    return corrected;
}

}  // namespace

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
        "SELECT id, owner_id, title, description, created_at, is_deleted, category, city, "
        "price_per_day FROM items WHERE id = ?";

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
        item.pricePerDay = column<double>(stmt, 8);
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
        "INSERT INTO items (owner_id, title, description, category, created_at, city, "
        "price_per_day) VALUES (?, ?, ?, ?, ?, ?, ?)";

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
    sqlite3_bind_double(stmt, 7, item.pricePerDay);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::insertItem: " + err);
    }
    sqlite3_finalize(stmt);
    addWordsToVocab(item.title);
    return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
}

bool ItemRepository::editItem(int itemId, const Item& newItem) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "UPDATE items SET title = ?, description = ?, category = ?, city = ?, price_per_day = ? "
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
    sqlite3_bind_double(stmt, 5, newItem.pricePerDay);
    sqlite3_bind_int(stmt, 6, itemId);

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::editItem: " + err);
    }

    int changed = sqlite3_changes(db_.handle());
    sqlite3_finalize(stmt);

    if (changed > 0) {
        addWordsToVocab(newItem.title);
    }
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

std::vector<std::string> ItemRepository::findCities() {
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT DISTINCT city FROM items WHERE is_deleted = 0 AND city != '' ORDER BY city";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::findCities: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    std::vector<std::string> cities;
    while (true) {
        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_DONE) break;
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("ItemRepository::findCities: " + err);
        }
        cities.push_back(column<std::string>(stmt, 0));
    }

    sqlite3_finalize(stmt);
    return cities;
}

std::vector<ItemShort> ItemRepository::searchByLike(int userId, const std::string& city,
                                                    const std::string& category,
                                                    const std::string& title, int limit,
                                                    int offset) {
    sqlite3_stmt* stmt = nullptr;
    const std::string sql =
        std::string("SELECT id, title, city ") + itemSearchWhere + "ORDER BY id LIMIT ? OFFSET ?";

    if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("searchByLike prepare: " +
                                 std::string(sqlite3_errmsg(db_.handle())));
    }

    bindItemSearch(stmt, userId, city, category, title);
    sqlite3_bind_int(stmt, 8, limit);
    sqlite3_bind_int(stmt, 9, offset);

    std::vector<ItemShort> items;
    while (true) {
        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_DONE) break;
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("ItemRepository::searchByLike step: " + err);
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

std::vector<ItemShort> ItemRepository::searchByFts(int userId, const std::string& city,
                                                   const std::string& category,
                                                   const std::string& title, int limit,
                                                   int offset) {
    if (title.empty()) {
        return {};
    }
    sqlite3_stmt* stmt = nullptr;
    const char* sql =
        "SELECT items.id, items.title, items.city FROM items "
        "JOIN items_title_fts ON items.id = items_title_fts.rowid "
        "WHERE items.is_deleted = 0 AND items.owner_id != ? "
        "AND (? = '' OR lower(items.city) = lower(?)) "
        "AND (? = '' OR items.category = ?) "
        "AND items_title_fts MATCH ? "
        "ORDER BY items_title_fts.rank, items.id "
        "LIMIT ? OFFSET ?";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("searchByFts prepare: " +
                                 std::string(sqlite3_errmsg(db_.handle())));
    }

    std::string matchQuery = quoteForFts(title);

    sqlite3_bind_int(stmt, 1, userId);
    sqlite3_bind_text(stmt, 2, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, category.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, matchQuery.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 7, limit);
    sqlite3_bind_int(stmt, 8, offset);

    std::vector<ItemShort> items;
    while (true) {
        int stepResult = sqlite3_step(stmt);
        if (stepResult == SQLITE_DONE) break;
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("ItemRepository::searchByFts step: " + err);
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

std::vector<ItemShort> ItemRepository::findItems(int userId, const std::string& city,
                                                 const std::string& category,
                                                 const std::string& title, int limit, int offset) {
    if (title.empty() || title.length() < 3) {
        return searchByLike(userId, city, category, title, limit, offset);
    }

    auto results = searchByFts(userId, city, category, title, limit, offset);
    if (!results.empty()) return results;

    std::string correctedTitle = correctTitle(db_, title);
    if (correctedTitle != title) {
        results = searchByFts(userId, city, category, correctedTitle, limit, offset);
        if (!results.empty()) return results;
    }

    return searchByLike(userId, city, category, title, limit, offset);
}

int ItemRepository::countItems(int userId, const std::string& city, const std::string& category,
                               const std::string& title) {
    auto countByLike = [&]() -> int {
        sqlite3_stmt* stmt = nullptr;
        const std::string sql = std::string("SELECT COUNT(*) ") + itemSearchWhere;
        if (sqlite3_prepare_v2(db_.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("ItemRepository::countItems LIKE prepare: " +
                                     std::string(sqlite3_errmsg(db_.handle())));
        }
        bindItemSearch(stmt, userId, city, category, title);

        int stepResult = sqlite3_step(stmt);
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("ItemRepository::countItems LIKE step: " + err);
        }

        int count = column<int>(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    };

    if (title.empty() || title.length() < 3) {
        return countByLike();
    }

    auto countByFts = [&](const std::string& searchTitle) -> int {
        sqlite3_stmt* stmt = nullptr;
        const char* sql =
            "SELECT COUNT(*) FROM items "
            "JOIN items_title_fts ON items.id = items_title_fts.rowid "
            "WHERE items.is_deleted = 0 AND items.owner_id != ? "
            "AND (? = '' OR lower(items.city) = lower(?)) "
            "AND (? = '' OR items.category = ?) "
            "AND items_title_fts MATCH ?";
        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("ItemRepository::countItems FTS prepare: " +
                                     std::string(sqlite3_errmsg(db_.handle())));
        }
        std::string matchQuery = quoteForFts(searchTitle);
        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_bind_text(stmt, 2, city.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, city.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, matchQuery.c_str(), -1, SQLITE_TRANSIENT);

        int stepResult = sqlite3_step(stmt);
        if (stepResult != SQLITE_ROW) {
            std::string err = sqlite3_errmsg(db_.handle());
            sqlite3_finalize(stmt);
            throw std::runtime_error("ItemRepository::countItems FTS step: " + err);
        }

        int count = column<int>(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    };

    int count = countByFts(title);
    if (count > 0) return count;

    std::string correctedTitle = correctTitle(db_, title);
    if (correctedTitle != title) {
        count = countByFts(correctedTitle);
        if (count > 0) return count;
    }

    return countByLike();
}


int ItemRepository::GetNotDeletedItemsCount() {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT COUNT(*) FROM items WHERE is_deleted = 0";

    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("ItemRepository::GetNotDeletedItemsCount: ") +
                                 sqlite3_errmsg(db_.handle()));
    }

    int stepResult = sqlite3_step(stmt);
    if (stepResult != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(db_.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("ItemRepository::GetNotDeletedItemsCount: " + err);
    }

    int count = column<int>(stmt, 0);
    sqlite3_finalize(stmt);
    return count;
}

void ItemRepository::populateTitleVocab() {
    sqlite3_stmt* checkStmt = nullptr;
    if (sqlite3_prepare_v2(db_.handle(), "SELECT COUNT(*) FROM title_vocab", -1, &checkStmt,
                           nullptr) == SQLITE_OK) {
        if (sqlite3_step(checkStmt) == SQLITE_ROW && column<int>(checkStmt, 0) > 0) {
            sqlite3_finalize(checkStmt);
            return;
        }
        sqlite3_finalize(checkStmt);
    }

    sqlite3_stmt* readStmt = nullptr;
    const char* readSql = "SELECT title FROM items WHERE is_deleted = 0";
    if (sqlite3_prepare_v2(db_.handle(), readSql, -1, &readStmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("populateTitleVocab (read): " +
                                 std::string(sqlite3_errmsg(db_.handle())));
    }

    std::set<std::string> words;
    while (sqlite3_step(readStmt) == SQLITE_ROW) {
        std::string title = column<std::string>(readStmt, 0);
        std::string current;
        for (unsigned char c : title) {
            if (std::isalpha(c)) {
                current.push_back(static_cast<char>(std::tolower(c)));
            } else if (!current.empty()) {
                words.insert(current);
                current.clear();
            }
        }
        if (!current.empty()) words.insert(current);
    }
    sqlite3_finalize(readStmt);

    sqlite3_stmt* insertStmt = nullptr;
    const char* insertSql = "INSERT OR IGNORE INTO title_vocab(word) VALUES (?)";
    if (sqlite3_prepare_v2(db_.handle(), insertSql, -1, &insertStmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("populateTitleVocab (insert): " +
                                 std::string(sqlite3_errmsg(db_.handle())));
    }

    for (const auto& w : words) {
        sqlite3_bind_text(insertStmt, 1, w.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(insertStmt);
        sqlite3_reset(insertStmt);
    }
    sqlite3_finalize(insertStmt);
}

void ItemRepository::addWordsToVocab(const std::string& title) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT OR IGNORE INTO title_vocab(word) VALUES (?)";
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return;

    std::string current;
    for (unsigned char c : title) {
        if (std::isalpha(c)) {
            current.push_back(static_cast<char>(std::tolower(c)));
        } else if (!current.empty()) {
            sqlite3_bind_text(stmt, 1, current.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
            current.clear();
        }
    }
    if (!current.empty()) {
        sqlite3_bind_text(stmt, 1, current.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
}

}  // namespace share