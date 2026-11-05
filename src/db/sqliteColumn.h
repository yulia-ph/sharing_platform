#pragma once

#include <sqlite3.h>

#include <optional>
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

template <>
inline std::optional<std::string> column<std::optional<std::string>>(sqlite3_stmt* stmt,
                                                                     int index) {
    if (sqlite3_column_type(stmt, index) == SQLITE_NULL) {
        return std::nullopt;
    }
    const auto* text = sqlite3_column_text(stmt, index);
    if (!text) return std::nullopt;
    return std::string(reinterpret_cast<const char*>(text));
}

template <>
inline double column<double>(sqlite3_stmt* stmt, int index) {
    return sqlite3_column_double(stmt, index);
}

template <>
inline std::optional<double> column<std::optional<double>>(sqlite3_stmt* stmt, int index) {
    if (sqlite3_column_type(stmt, index) == SQLITE_NULL) {
        return std::nullopt;
    }
    return sqlite3_column_double(stmt, index);
}

}  // namespace share
