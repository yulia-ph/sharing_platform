#include <iostream>

#include "AuthService.h"
#include "BookingService.h"
#include "Database.h"
#include "HttpServer.h"
#include "ItemService.h"
#include "Migrations.h"

int main() {
    using namespace share;

    try {
        Database db("shared.db");
        runMigrations(db);
        UserRepository users(db);
        BookingRepository bookingsRepo(db);
        ItemRepository itemsRepo(db);
        AuthService auth(users);
        ItemService items(itemsRepo, bookingsRepo, db);
        BookingService bookings(bookingsRepo);

        HttpServer server(auth, items, bookings);
        server.run(8080);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}