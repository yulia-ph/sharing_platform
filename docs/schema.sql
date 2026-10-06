CREATE TABLE users (
    id            INTEGER PRIMARY KEY AUTOINCREMENT,
    username      TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL
);

CREATE TABLE items (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    owner_id    INTEGER NOT NULL REFERENCES users(id),
    title       TEXT NOT NULL,
    description TEXT,
    category    TEXT NOT NULL DEFAULT 'other',
    created_at  TEXT NOT NULL DEFAULT (date('now')),
    is_deleted  INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE bookings (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    item_id     INTEGER NOT NULL REFERENCES items(id),
    borrower_id INTEGER NOT NULL REFERENCES users(id),
    owner_id    INTEGER NOT NULL REFERENCES users(id),
    start_date  TEXT NOT NULL,
    end_date    TEXT NOT NULL,
    status      TEXT NOT NULL DEFAULT 'reserved'
        CHECK (status IN ('reserved','active','returned','cancelled')),
    returned_at TEXT,
    created_at  TEXT NOT NULL DEFAULT (date('now')),
    CHECK (start_date <= end_date)
);

CREATE INDEX idx_items_owner
    ON items(owner_id);
CREATE INDEX idx_items_category
    ON items(category);
CREATE INDEX idx_bookings_item_dates
    ON bookings(item_id, start_date, end_date);
CREATE INDEX idx_bookings_borrower
    ON bookings(borrower_id);