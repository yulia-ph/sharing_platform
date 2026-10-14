#include "HttpServer.h"

#include <httplib.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>

namespace share {

static std::string generateSessionId() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    const char* hex = "0123456789abcdef";
    std::string id;
    for (int i = 0; i < 32; ++i) id += hex[dis(gen)];
    return id;
}

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

HttpServer::HttpServer(AuthService& auth) : auth_(auth) {}

void HttpServer::handleRegister(const std::string& body, std::string& outBody, int& outStatus,
                                std::string& outContentType) {
    outContentType = "application/json";

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(body);
    } catch (...) {
        outStatus = 400;
        outBody = R"({"ok":false,"message":"Invalid JSON"})";
        return;
    }

    std::string username = data.value("username", "");
    std::string password = data.value("password", "");
    std::string passwordDouble = data.value("passwordDouble", "");

    Result r = auth_.registerUser(username, password, passwordDouble);

    nlohmann::json response = {{"ok", r.ok}, {"message", r.message}};
    outStatus = r.ok ? 200 : 400;
    outBody = response.dump();
}

void HttpServer::handleLogin(const std::string& body, std::string& outBody, int& outStatus,
                             std::string& outContentType, std::string& outSetCookie) {
    outContentType = "application/json";

    nlohmann::json data;
    try {
        data = nlohmann::json::parse(body);
    } catch (...) {
        outStatus = 400;
        outBody = R"({"ok":false,"message":"Invalid JSON"})";
        return;
    }

    std::string username = data.value("username", "");
    std::string password = data.value("password", "");

    auto userId = auth_.login(username, password);

    if (!userId) {
        outStatus = 401;
        outBody = R"({"ok":false,"message":"Wrong username or password"})";
        return;
    }

    std::string sessionId = generateSessionId();
    sessions_[sessionId] = *userId;

    outSetCookie = "sessionId=" + sessionId + "; Path=/; HttpOnly";

    outStatus = 200;
    outBody = R"({"ok":true})";
}

void HttpServer::run(int port) {
    httplib::Server svr;

    svr.Get("/", [](const auto&, auto& res) {
        std::string html = readFile("public/index.html");
        if (html.empty()) {
            res.status = 404;
            res.set_content("index.html not found", "text/plain");
            return;
        }
        res.set_content(html, "text/html");
    });

    svr.Get("/app.js", [](const auto&, auto& res) {
        std::string js = readFile("public/app.js");
        if (js.empty()) {
            res.status = 404;
            res.set_content("app.js not found", "text/plain");
            return;
        }
        res.set_content(js, "application/javascript");
    });

    svr.Post("/api/register", [this](const auto& req, auto& res) {
        std::string body;
        int status = 200;
        std::string contentType;

        handleRegister(req.body, body, status, contentType);

        res.status = status;
        res.set_content(body, contentType);
    });

    svr.Post("/api/login", [this](const auto& req, auto& res) {
        std::string body;
        int status = 200;
        std::string contentType;
        std::string setCookie;

        handleLogin(req.body, body, status, contentType, setCookie);

        res.status = status;
        if (!setCookie.empty()) {
            res.set_header("Set-Cookie", setCookie);
        }
        res.set_content(body, contentType);
    });

    svr.listen("localhost", port);
}

}  // namespace share