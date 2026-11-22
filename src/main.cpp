#include <sqlite3.h>

#include <iostream>

#include "AuthService.h"
#include "BookingService.h"
#include "Database.h"
#include "HttpServer.h"
#include "ItemService.h"
#include "Migrations.h"

extern "C" int sqlite3_spellfix_init(sqlite3* db, char** pzErrMsg,
                                     const sqlite3_api_routines* pApi);

int main() {
    using namespace share;
    sqlite3_auto_extension(reinterpret_cast<void (*)()>(sqlite3_spellfix_init));

    try {
        Database db("shared.db");
        runMigrations(db);
        UserRepository users(db);
        BookingRepository bookingsRepo(db);
        ItemRepository itemsRepo(db);
        itemsRepo.populateTitleVocab();
        AuthService auth(users);
        ItemService items(itemsRepo, bookingsRepo, db);
        BookingService bookings(bookingsRepo, itemsRepo);

        HttpServer server(auth, items, bookings);
        server.run(8080);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}