#include <iostream>

#include "AuthService.h"
#include "ConsoleApp.h"
#include "Database.h"
#include "Migrations.h"
#include "UserRepository.h"

int main() {
    using namespace share;

    try {
        Database db("shared.db");
        runMigrations(db);
        UserRepository users(db);
        AuthService auth(users);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}