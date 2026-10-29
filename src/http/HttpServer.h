#pragma once

#include <string>

#include "AuthService.h"
#include "BookingService.h"
#include "Enums.h"
#include "ItemService.h"

namespace share {

class HttpServer {
   public:
    HttpServer(AuthService& auth, ItemService& items, BookingService& bookings);

    void run(int port);

   private:
    AuthService& auth_;
    ItemService& items_;
    BookingService& bookings_;

    void handleRegister(const std::string& body, std::string& outBody, int& outStatus,
                        std::string& outContentType);

    void handleLogin(const std::string& body, std::string& outBody, int& outStatus,
                     std::string& outContentType, std::string& outSetCookie);
};

}  // namespace share
