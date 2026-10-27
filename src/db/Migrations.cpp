#include "Migrations.h"
#include "sqliteColumn.h"

#include <sqlite3.h>
#include <stdexcept>

#include <string>

namespace share {

static void ensureCityColumn(Database& db) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db.handle(), "PRAGMA table_info(items)", -1, &stmt, nullptr) !=
        SQLITE_OK) {
        throw std::runtime_error(std::string("Migration error (prepare): ") +
                                 sqlite3_errmsg(db.handle()));
    }

    bool hasCity = false;
    int stepResult;

    while ((stepResult = sqlite3_step(stmt)) == SQLITE_ROW) {
        std::string colName = column<std::string>(stmt, 1);

        if (colName == "city") {
            hasCity = true;
            break;
        }
    }

    if (stepResult != SQLITE_DONE && stepResult != SQLITE_ROW) {
        std::string err = sqlite3_errmsg(db.handle());
        sqlite3_finalize(stmt);
        throw std::runtime_error("Migration error (step): " + err);
    }

    sqlite3_finalize(stmt);

    if (!hasCity) {
        db.exec("ALTER TABLE items ADD COLUMN city TEXT NOT NULL DEFAULT ''");
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
                CHECK (status IN ('reserved','active','returned','cancelled')),
            returned_at TEXT,
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
}

}  // namespace share
