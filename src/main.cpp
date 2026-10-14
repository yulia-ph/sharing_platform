#include <iostream>

#include "AuthService.h"
#include "Database.h"
#include "HttpServer.h"
#include "Migrations.h"
#include "UserRepository.h"

int main() {
    using namespace share;

    try {
        Database db("shared.db");
        runMigrations(db);
        UserRepository users(db);
        AuthService auth(users);

        HttpServer server(auth);
        server.run(8080);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}