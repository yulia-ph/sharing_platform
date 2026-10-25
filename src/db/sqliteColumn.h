#pragma once

#include <sqlite3.h>

#include <string>

namespace share {

template <typename T>
T column(sqlite3_stmt* stmt, int index);

template <>
inline int column<int>(sqlite3_stmt* stmt, int index) {
    return sqlite3_column_int(stmt, index);
}

template <>
inline std::string column<std::string>(sqlite3_stmt* stmt, int index) {
    const auto* text = sqlite3_column_text(stmt, index);
    if (!text) return "";
    return reinterpret_cast<const char*>(text);
}

}  // namespace share
