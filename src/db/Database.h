#pragma once

#include <sqlite3.h>

#include <string>

namespace share {

class Database {
   public:
    explicit Database(const std::string& path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* handle() {
        return db_;
    }

    void exec(const std::string& sql);

    void beginTransaction();
    void commit();
    void rollback();

   private:
    sqlite3* db_ = nullptr;
};

}  // namespace share