#include "PasswordHasher.h"

#include <sodium.h>

#include <stdexcept>

namespace share {

std::string hashPassword(const std::string& password) {
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium init failed");
    }

    char hashed[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(hashed, password.c_str(), password.size(),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        throw std::runtime_error("password hashing failed");
    }

    return std::string(hashed);
}

bool verifyPassword(const std::string& password, const std::string& storedHash) {
    if (sodium_init() < 0) {
        return false;
    }
    return crypto_pwhash_str_verify(storedHash.c_str(), password.c_str(), password.size()) == 0;
}

}  // namespace share