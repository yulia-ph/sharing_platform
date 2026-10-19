#include "HttpServer.h"

#include <httplib.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <random>
#include <sstream>

#include "AuthService.h"

namespace share {

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::stringstream buf;
    buf << f.rdbuf();
    return buf.str();
}

static std::string getCookie(const httplib::Request& req, const std::string& name) {
    if (!req.has_header("Cookie")) return "";

    std::string cookies = req.get_header_value("Cookie");
    std::string prefix = name + "=";

    size_t pos = cookies.find(prefix);
    if (pos == std::string::npos) return "";

    pos += prefix.size();
    size_t end = cookies.find(';', pos);
    if (end == std::string::npos) return cookies.substr(pos);
    return cookies.substr(pos, end - pos);
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

    std::string sessionId = auth_.createSession(*userId);

    outSetCookie = "sessionId=" + sessionId + "; Path=/; HttpOnly";

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

    svr.Get("/style.css", [](const auto&, auto& res) {
        std::string css = readFile("public/style.css");
        if (css.empty()) {
            res.status = 404;
            res.set_content("style.css not found", "text/plain");
            return;
        }
        res.set_content(css, "text/css");
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

    svr.Post("/api/logout", [this](const auto& req, auto& res) {
        std::string sessionId = getCookie(req, "sessionId");

        auth_.logoutUser(sessionId);

        res.set_header("Set-Cookie", "sessionId=; Path=/; HttpOnly; Max-Age=0");
        res.set_content(R"({"ok":true})", "application/json");
    });

    svr.listen("localhost", port);
}

}  // namespace share