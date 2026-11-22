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
    city        TEXT NOT NULL DEFAULT '',
    price_per_day REAL NOT NULL DEFAULT 0,
    created_at  TEXT NOT NULL DEFAULT (date('now')),
    is_deleted  INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE bookings (
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
    notice      TEXT, 
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

CREATE VIRTUAL TABLE items_title_fts USING fts5(
    title,
    content='items',
    content_rowid='id',
    tokenize='porter unicode61'
);

CREATE TRIGGER items_title_ai AFTER INSERT ON items BEGIN
    INSERT INTO items_title_fts(rowid, title)
    VALUES (new.id, new.title);
END;

CREATE TRIGGER items_title_ad AFTER DELETE ON items BEGIN
    INSERT INTO items_title_fts(items_title_fts, rowid, title)
    VALUES('delete', old.id, old.title);
END;

CREATE TRIGGER items_title_au AFTER UPDATE OF title ON items BEGIN
    INSERT INTO items_title_fts(items_title_fts, rowid, title)
    VALUES('delete', old.id, old.title);
    INSERT INTO items_title_fts(rowid, title)
    VALUES (new.id, new.title);
END;

INSERT INTO items_title_fts(items_title_fts) VALUES('rebuild');

CREATE VIRTUAL TABLE title_vocab USING spellfix1;