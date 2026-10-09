#pragma once

#include <cctype>
#include <optional>

#include "Entities.h"
#include "PasswordHasher.h"
#include "Result.h"
#include "UserRepository.h"

namespace share {

class AuthService {
   public:
    explicit AuthService(UserRepository& users) : users_(users) {}

    Result registerUser(const std::string& username, const std::string& password,
                        const std::string& passwordDouble);
    std::optional<int> login(const std::string& username, const std::string& password);

   private:
    UserRepository& users_;
};
}  // namespace share