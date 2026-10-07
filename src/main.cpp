#include <iostream>

#include "ConsoleApp.h"
#include "Database.h"
#include "Migrations.h"
#include "UserRepository.h"

int main() {
    using namespace share;
    Database db("shared.db");
    runMigrations(db);
    UserRepository users(db);

    return 0;
}