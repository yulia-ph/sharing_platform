#include "Database.h"
#include <string>
#include <stdexcept>

namespace share {

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string msg = db_ ? sqlite3_errmsg(db_) : "unknown error";
        if (db_) sqlite3_close(db_);
        db_ = nullptr;
        throw std::runtime_error("Unable to open database file: " + msg);
    }
    exec("PRAGMA foreign_keys = ON;");
    exec("PRAGMA journal_mode = WAL;");
    exec("PRAGMA busy_timeout = 5000;");
}

Database::~Database() {
    if (db_) sqlite3_close(db_);
}

void Database::exec(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error("SQL error: " + msg + "\nQuery: " + sql);
    }
}

void Database::beginTransaction() {
    exec("BEGIN IMMEDIATE;");
}
void Database::commit() {
    exec("COMMIT;");
}
void Database::rollback() {
    exec("ROLLBACK;");
}

}  // namespace share