#include "Migrations.h"

namespace share {

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
            category    TEXT NOT NULL DEFAULT 'other'
            CHECK (category IN (
            'electronics', 'toys', 'for kitchen', 'sports', 'books', 'leisure', 'tools', 'services', 'other'
            )),
            created_at  TEXT NOT NULL DEFAULT CURRENT_DATE,
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
            created_at  TEXT NOT NULL DEFAULT CURRENT_DATE,
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
}

}  // namespace share
