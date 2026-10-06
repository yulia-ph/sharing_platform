#include "UserRepository.h"

#include <sqlite3.h>
#include <stdexcept>

namespace share {

    std::optional<User> UserRepository::findById(int id) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT id, username, password_hash FROM users WHERE id = ?";
        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("UserRepository::findById: ошибка prepare");
        }

        sqlite3_bind_int(stmt, 1, id);
        std::optional<User> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            User user;
            user.id=sqlite3_column_int(stmt, 0);
            user.username=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            user.passwordHash=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            result=user;
        }
        sqlite3_finalize(stmt);
        return result;
    }

    std::optional<User> UserRepository::findByUsername(const std::string& username) {
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "SELECT id, username, password_hash FROM users WHERE username = ?";
        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("UserRepository::findByUsername: ошибка prepare");
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        std::optional<User> result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            User user;
            user.id=sqlite3_column_int(stmt, 0);
            user.username=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            user.passwordHash=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            result=user;
        }
        sqlite3_finalize(stmt);
        return result;
    }

    bool UserRepository::existsUsername(const std::string& username) {
        sqlite3_stmt* stmt=nullptr;
        const char* sql="SELECT 1 FROM users WHERE username = ? LIMIT 1";
        if(sqlite3_prepare_v2(db_.handle(),sql,-1,&stmt, nullptr)!=SQLITE_OK){
            throw std::runtime_error("UserRepository::existsUsername: ошибка prepare");
        }

        sqlite3_bind_text(stmt,1,username.c_str(),-1,SQLITE_TRANSIENT);
        bool exists=(sqlite3_step(stmt)==SQLITE_ROW);
        sqlite3_finalize(stmt);
        return exists;        
    }

    int UserRepository::insert(const User& user){
        sqlite3_stmt* stmt = nullptr;
        const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";

        if (sqlite3_prepare_v2(db_.handle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("UserRepository::insert: ошибка prepare");
        }

        sqlite3_bind_text(stmt, 1, user.username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user.passwordHash.c_str(), -1, SQLITE_TRANSIENT);

        int newUser = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (newUser != SQLITE_DONE) {
        return -1;
        }
        return static_cast<int>(sqlite3_last_insert_rowid(db_.handle()));
    }
}