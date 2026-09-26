#pragma once

#include <optional>

#include "Entities.h"
#include "Result.h"

namespace share {

class AuthService {
public:
    Result registerUser(const std::string& username,
                        const std::string& password,
                        const std::string& passwordDuble);
    std::optional<User> login(const std::string& username,
                              const std::string& password);
};
}