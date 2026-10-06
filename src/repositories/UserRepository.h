#pragma once

#include <optional>

#include "Database.h"
#include "Entities.h"

namespace share {

class UserRepository {
   public:
    explicit UserRepository(Database& db) : db_(db) {}
    std::optional<User> findById(int id);
    std::optional<User> findByUsername(const std::string& username);
    bool existsUsername(const std::string& username);
    int insert(const User& user);

   private:
    Database& db_;
};

}  