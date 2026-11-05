#include "Migrations.h"

#include <sqlite3.h>

#include <stdexcept>
#include <string>

#include "sqliteColumn.h"

namespace share {

static bool tableHasColumn(Database& db, const char* table, const std::string& columnName) {
    sqlite3_stmt* stmt = nullptr;
    const std::string sql = std::string("PRAGMA table_info(") + table + ")";
    if (sqlite3_prepare_v2(db.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Migration error (prepare columns): ") +
                                 sqlite3_errmsg(db.handle()));
    }

    bool found = false;
    int stepResult = SQLITE_DONE;
    while ((stepResult = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (column<std::string>(stmt, 1) == columnName) {
            found = true;
            break;
        }
    }
    if (stepResult != SQLITE_DONE && stepResult != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(db.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("Migration error (read columns): " + err);
    }
    sqlite3_finalize(stmt);
    return found;
}

static std::string columnDeclaredType(Database& db, const char* table,
                                      const std::string& columnName) {
    sqlite3_stmt* stmt = nullptr;
    const std::string sql = std::string("PRAGMA table_info(") + table + ")";
    if (sqlite3_prepare_v2(db.handle(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Migration error (prepare column type): ") +
                                 sqlite3_errmsg(db.handle()));
    }

    std::string type;
    int stepResult = SQLITE_DONE;
    while ((stepResult = sqlite3_step(stmt)) == SQLITE_ROW) {
        if (column<std::string>(stmt, 1) == columnName) {
            type = column<std::string>(stmt, 2);
            break;
        }
    }
    if (stepResult != SQLITE_DONE && stepResult != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(db.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("Migration error (read column type): " + err);
    }
    sqlite3_finalize(stmt);
    return type;
}

static void ensureCityColumn(Database& db) {
    if (!tableHasColumn(db, "items", "city")) {
        db.exec("ALTER TABLE items ADD COLUMN city TEXT NOT NULL DEFAULT ''");
    }
}

static void ensureOverdueStatus(Database& db) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT sql FROM sqlite_master WHERE type = 'table' AND name = 'bookings'";
    if (sqlite3_prepare_v2(db.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Migration error (prepare bookings): ") +
                                 sqlite3_errmsg(db.handle()));
    }

    std::string tableSql;
    int stepResult = sqlite3_step(stmt);
    if (stepResult == SQLITE_ROW) tableSql = column<std::string>(stmt, 0);
    sqlite3_finalize(stmt);

    if (stepResult != SQLITE_ROW && stepResult != SQLITE_DONE) {
        throw std::runtime_error(std::string("Migration error (read bookings): ") +
                                 sqlite3_errmsg(db.handle()));
    }
    if (tableSql.find("overdue") != std::string::npos) return;

    db.beginTransaction();
    try {
        db.exec(R"(
            CREATE TABLE bookings_new (
                id          INTEGER PRIMARY KEY AUTOINCREMENT,
                item_id     INTEGER NOT NULL REFERENCES items(id),
                borrower_id INTEGER NOT NULL REFERENCES users(id),
                start_date  TEXT NOT NULL,
                end_date    TEXT NOT NULL,
                status      TEXT NOT NULL DEFAULT 'reserved'
                    CHECK (status IN ('reserved','active','returned','cancelled','overdue')),
                returned_at TEXT,
                created_at  TEXT NOT NULL DEFAULT (date('now')),
                CHECK (start_date <= end_date)
            );

            INSERT INTO bookings_new (
                id, item_id, borrower_id, start_date, end_date, status, returned_at, created_at
            )
            SELECT id, item_id, borrower_id, start_date, end_date, status, returned_at, created_at
            FROM bookings;

            DROP TABLE bookings;
            ALTER TABLE bookings_new RENAME TO bookings;

            CREATE INDEX idx_bookings_item_dates
                ON bookings(item_id, start_date, end_date);
            CREATE INDEX idx_bookings_borrower
                ON bookings(borrower_id);
        )");
        db.commit();
    } catch (...) {
        db.rollback();
        throw;
    }
}

static void ensureMoneyColumns(Database& db) {
    db.beginTransaction();
    try {
        if (!tableHasColumn(db, "items", "price_per_day")) {
            db.exec("ALTER TABLE items ADD COLUMN price_per_day REAL NOT NULL DEFAULT 0");
        } else if (columnDeclaredType(db, "items", "price_per_day") != "REAL") {
            db.exec("ALTER TABLE items RENAME COLUMN price_per_day TO price_per_day_old");
            db.exec("ALTER TABLE items ADD COLUMN price_per_day REAL NOT NULL DEFAULT 0");
            db.exec("UPDATE items SET price_per_day = price_per_day_old");
            db.exec("ALTER TABLE items DROP COLUMN price_per_day_old");
        }

        if (!tableHasColumn(db, "bookings", "paid")) {
            db.exec("ALTER TABLE bookings ADD COLUMN paid INTEGER NOT NULL DEFAULT 0");
        }

        if (!tableHasColumn(db, "bookings", "amount")) {
            db.exec("ALTER TABLE bookings ADD COLUMN amount REAL");
            if (tableHasColumn(db, "bookings", "amount_quarters")) {
                db.exec(
                    "UPDATE bookings SET amount = amount_quarters / 4.0 "
                    "WHERE amount_quarters IS NOT NULL");
            }
        }
        if (tableHasColumn(db, "bookings", "amount_quarters")) {
            db.exec("ALTER TABLE bookings DROP COLUMN amount_quarters");
        }

        db.commit();
    } catch (...) {
        db.rollback();
        throw;
    }
}

void runMigrations(Database& db) {
    db.exec(R"(
        CREATE TABLE IF NOT EXISTS users (
            id            INTEGER PRIMARY KEY AUTOINCREMENT,
            username      TEXT NOT NULL UNIQUE,
            password_hash TEXT NOT NULL
        );

        CREATE TABLE IF NOT EXISTS items (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            owner_id    INTEGER NOT NULL REFERENCES users(id),
            title       TEXT NOT NULL,
            description TEXT,
            category    TEXT NOT NULL DEFAULT 'other',
            city        TEXT NOT NULL DEFAULT '',
            price_per_day REAL NOT NULL DEFAULT 0,
            created_at  TEXT NOT NULL DEFAULT (date('now')),
            is_deleted  INTEGER NOT NULL DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS bookings (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            item_id     INTEGER NOT NULL REFERENCES items(id),
            borrower_id INTEGER NOT NULL REFERENCES users(id),
            start_date  TEXT NOT NULL,
            end_date    TEXT NOT NULL,
            status      TEXT NOT NULL DEFAULT 'reserved'
                CHECK (status IN ('reserved','active','returned','cancelled','overdue')),
            returned_at TEXT,
            amount      REAL,
            paid        INTEGER NOT NULL DEFAULT 0,
            created_at  TEXT NOT NULL DEFAULT (date('now')),
            CHECK (start_date <= end_date)
        );

        CREATE INDEX IF NOT EXISTS idx_items_owner
            ON items(owner_id);
        CREATE INDEX IF NOT EXISTS idx_items_category
            ON items(category);
        CREATE INDEX IF NOT EXISTS idx_bookings_item_dates
            ON bookings(item_id, start_date, end_date);
        CREATE INDEX IF NOT EXISTS idx_bookings_borrower
            ON bookings(borrower_id);
        )");
    ensureCityColumn(db);
    ensureOverdueStatus(db);
    ensureMoneyColumns(db);
}

}  // namespace share