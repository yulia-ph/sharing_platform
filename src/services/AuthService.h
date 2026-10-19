#pragma once

#include <cctype>
#include <map>
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
    void logoutUser(const std::string& sessionId);
    std::string createSession(int userId);

   private:
    UserRepository& users_;
    std::map<std::string, int> sessions_;
};
}  // namespace share