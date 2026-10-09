#include "AuthService.h"

namespace share {

Result AuthService::registerUser(const std::string& username, const std::string& password,
                                 const std::string& passwordDouble) {
    if (username.size() < 3 || username.size() > 20) {
        return Result::error("Username must be from 3 to 20 characters");
    }

    for (char i : username) {
        if (!std::isalnum(static_cast<unsigned char>(i)) && i != '_') {
            return Result::error("Username must be all letters, numbers and _");
        }
    }

    if (users_.existsUsername(username)) {
        return Result::error("This username is already taken");
    }

    if (password != passwordDouble) {
        return Result::error("Typo in the second password");
    }

    if (password.size() < 6) {
        return Result::error("Password must be at least 6 characters");
    }

    User user;
    user.username = username;
    user.passwordHash = hashPassword(password);

    users_.insert(user);

    return Result::success();
}

std::optional<int> AuthService::login(const std::string& username, const std::string& password) {
    auto user = users_.findByUsername(username);
    if (!user) return std::nullopt;

    if (!verifyPassword(password, user->passwordHash)) {
        return std::nullopt;
    }

    return user->id;
}

}  // namespace share