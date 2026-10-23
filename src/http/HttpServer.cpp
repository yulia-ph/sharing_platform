#include "HttpServer.h"

#include <httplib.h>

#include <filesystem>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#include "AuthService.h"
#include "BookingService.h"
#include "Enums.h"
#include "ItemService.h"

namespace share {

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

static void sendJson(httplib::Response& res, int status, const std::string& body) {
    res.status = status;
    res.set_content(body, "application/json");
}

static void sendError(httplib::Response& res, int status, const std::string& message) {
    sendJson(res, status, nlohmann::json{{"ok", false}, {"message", message}}.dump());
}

static void sendResult(httplib::Response& res, const Result& result, bool withId) {
    if (result.ok) {
        nlohmann::json body = {{"ok", true}};
        if (withId) body["id"] = result.id;
        sendJson(res, 200, body.dump());
        return;
    }

    if (result.message == "Item not found") {
        sendError(res, 404, result.message);
        return;
    }
    if (result.message == "You don't have access to this item") {
        sendError(res, 403, result.message);
        return;
    }
    if (result.message == "Database error" || result.message.rfind("Failed", 0) == 0) {
        sendError(res, 500, "Internal server error");
        return;
    }
    sendError(res, 400, result.message);
}

static std::optional<int> queryId(const httplib::Request& req, const char* name) {
    if (!req.has_param(name)) return std::nullopt;

    const std::string value = req.get_param_value(name);
    if (value.empty()) return std::nullopt;

    try {
        size_t used = 0;
        int id = std::stoi(value, &used);
        if (used != value.size()) return std::nullopt;
        return id;
    } catch (...) {
        return std::nullopt;
    }
}

static std::optional<int> currentUser(AuthService& auth, const httplib::Request& req,
                                      httplib::Response& res) {
    auto userId = auth.userIdBySession(getCookie(req, "sessionId"));
    if (!userId) sendError(res, 401, "Not logged in");
    return userId;
}

HttpServer::HttpServer(AuthService& auth, ItemService& items, BookingService& bookings)
    : auth_(auth), items_(items), bookings_(bookings) {}

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

    outStatus = r.ok ? 200 : 400;
    if (r.ok) {
        outBody = R"({"ok":true})";
    } else {
        outBody = nlohmann::json{{"ok", false}, {"message", r.message}}.dump();
    }
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

    outStatus = 200;
    outBody = R"({"ok":true})";
}

static std::string findPublicDir() {
    namespace fs = std::filesystem;

    std::vector<fs::path> candidates = {
        "public",
        fs::path("..") / "public",
        fs::path("..") / ".." / "public",
    };

    std::error_code ec;
    fs::path exeDir;
#if defined(__APPLE__)
    char buffer[4096];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        exeDir = fs::weakly_canonical(buffer, ec).parent_path();
    }
#elif defined(__linux__)
    exeDir = fs::weakly_canonical("/proc/self/exe", ec).parent_path();
#endif
    if (!ec && !exeDir.empty()) {
        candidates.push_back(exeDir / "public");
        candidates.push_back(exeDir / ".." / "public");
        candidates.push_back(exeDir / ".." / ".." / "public");
    }

    for (const fs::path& candidate : candidates) {
        if (fs::is_regular_file(candidate / "index.html", ec)) {
            return fs::absolute(candidate).string();
        }
    }

    return "public";
}

void HttpServer::run(int port) {
    httplib::Server svr;

    const std::string publicDir = findPublicDir();
    if (!svr.set_mount_point("/", publicDir)) {
        std::cerr << "Cannot serve files from " << publicDir << "\n";
    } else {
        std::cout << "Serving files from " << publicDir << "\n";
    }

    svr.set_exception_handler([](const auto&, auto& res, std::exception_ptr pointer) {
        try {
            if (pointer) std::rethrow_exception(pointer);
        } catch (const std::exception& e) {
            std::cerr << e.what() << "\n";
        }
        sendError(res, 500, "Internal server error");
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

    svr.Get("/api/my-items", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        nlohmann::json list = nlohmann::json::array();
        for (const ItemShort& item : items_.findUserItems(*userId)) {
            list.push_back({{"id", item.id},
                            {"title", item.title},
                            {"city", item.city},
                            {"status", items_.availability(item.id)}});
        }
        sendJson(res, 200, list.dump());
    });

    svr.Get("/api/items", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        auto itemId = queryId(req, "id");
        if (!itemId || *itemId <= 0) {
            sendError(res, 400, "Invalid item id");
            return;
        }

        auto item = items_.findItemInfo(*itemId);
        if (!item || item->isDeleted) {
            sendError(res, 404, "Item not found");
            return;
        }

        nlohmann::json body = {{"id", item->id},
                               {"title", item->title},
                               {"description", item->description},
                               {"category", itemCategoryToString(item->category)},
                               {"city", item->city},
                               {"status", items_.availability(item->id)}};
        sendJson(res, 200, body.dump());
    });

    svr.Post("/api/items", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        nlohmann::json data;
        try {
            data = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        Result result =
            items_.addItem(*userId, data.value("title", ""), data.value("description", ""),
                           data.value("category", ""), data.value("city", ""));
        sendResult(res, result, true);
    });

    svr.Put("/api/items", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        nlohmann::json data;
        try {
            data = nlohmann::json::parse(req.body);
        } catch (...) {
            sendError(res, 400, "Invalid JSON");
            return;
        }

        int itemId = 0;
        try {
            itemId = data.at("id").get<int>();
        } catch (...) {
            sendError(res, 400, "Invalid item id");
            return;
        }

        Result result =
            items_.editItem(*userId, itemId, data.value("title", ""), data.value("description", ""),
                            data.value("category", ""), data.value("city", ""));
        sendResult(res, result, false);
    });

    svr.Delete("/api/items", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        auto itemId = queryId(req, "id");
        if (!itemId || *itemId <= 0) {
            sendError(res, 400, "Invalid item id");
            return;
        }

        sendResult(res, items_.deleteItem(*userId, *itemId), false);
    });

    svr.Get("/api/bookings", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        auto itemId = queryId(req, "itemId");
        if (!itemId || *itemId <= 0) {
            sendError(res, 400, "Invalid item id");
            return;
        }

        nlohmann::json list = nlohmann::json::array();
        for (const BookingShort& booking : bookings_.findShortBookingsOfItem(*itemId)) {
            list.push_back({{"id", booking.id},
                            {"borrowerUsername", booking.borrowerUsername},
                            {"startDate", booking.startDate},
                            {"endDate", booking.endDate},
                            {"status", bookingStatusToString(booking.status)}});
        }
        sendJson(res, 200, list.dump());
    });

    svr.Get("/api/categories", [this](const auto& req, auto& res) {
        auto userId = currentUser(auth_, req, res);
        if (!userId) return;

        sendJson(res, 200, nlohmann::json(items_.getCategories()).dump());
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