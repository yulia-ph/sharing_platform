#pragma once

#include <map>
#include <string>

#include "AuthService.h"

namespace share {

class HttpServer {
   public:
    HttpServer(AuthService& auth);

    void run(int port);

   private:
    AuthService& auth_;

    std::map<std::string, int> sessions_;

    void handleRegister(const std::string& body, std::string& outBody, int& outStatus,
                        std::string& outContentType);

    void handleLogin(const std::string& body, std::string& outBody, int& outStatus,
                     std::string& outContentType, std::string& outSetCookie);
};

}  // namespace share