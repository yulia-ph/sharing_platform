#pragma once

#include <string>

namespace share {

class PasswordHasher {
public:
    // Хеширует пароль. Соль генерируется автоматически внутри libsodium
    // и сохраняется вместе с хешем в одной строке.
    // Возвращает строку вида:
    // $argon2id$v=19$m=65536,t=2,p=1$c29tZXNhbHQ$...
    // Эту строку и кладём в поле User::passwordHash.
    static std::string hashPassword(const std::string& password);

    // Проверяет, соответствует ли пароль сохранённому хешу.
    // Возвращает true, если пароль верный.
    static bool verifyPassword(const std::string& password,
                               const std::string& storedHash);
};

}