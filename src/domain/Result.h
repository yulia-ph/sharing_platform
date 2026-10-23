#pragma once

#include <string>

namespace share {

struct Result {
    bool ok = true;
    std::string message;
    int id = 0;

    static Result success(int id = 0) {
        Result result;
        result.id = id;
        return result;
    }

    static Result error(const std::string& msg) {
        return {false, msg};
    }
};

}  // namespace share