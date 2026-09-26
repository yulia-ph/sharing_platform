#pragma once

#include <string>

namespace share {

struct Result {
    bool ok = true;
    std::string message;

    static Result success() {
        return {true, ""};
    }

    static Result error(const std::string& msg) {
        return {false, msg};
    }
};

}