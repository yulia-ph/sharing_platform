#pragma once

#include <optional>

#include "Entities.h"

namespace share {

class UserRepository {
public:
    std::optional<User> findByUsername(const std::string& username);
    int insert(const User& user);
};
}