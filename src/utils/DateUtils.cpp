#include "DateUtils.h"

namespace share {

std::string today() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);

    char buf[11];  // "YYYY-MM-DD" + '\0'
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &tm);
    return std::string(buf);
}

bool isValidDate(const std::string& s) {
    if (s.size() != 10) return false;

    if (s[4] != '-' || s[7] != '-') return false;
    for (int i : {0, 1, 2, 3, 5, 6, 8, 9}) {
        if (!std::isdigit(s[i])) return false;
    }

    int year = std::stoi(s.substr(0, 4));
    int month = std::stoi(s.substr(5, 2));
    int day = std::stoi(s.substr(8, 2));

    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))) {
        daysInMonth[1] = 29;
    }
    if (day < 1 || day > daysInMonth[month - 1]) return false;

    return true;
}

int inclusiveDays(const std::string& start, const std::string& end) {
    if (!isValidDate(start) || !isValidDate(end) || end < start) return 0;

    auto serial = [](const std::string& date) {
        int year = std::stoi(date.substr(0, 4));
        int month = std::stoi(date.substr(5, 2));
        int day = std::stoi(date.substr(8, 2));
        year -= month <= 2;
        const int era = (year >= 0 ? year : year - 399) / 400;
        const unsigned yearOfEra = static_cast<unsigned>(year - era * 400);
        const unsigned dayOfYear = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
        const unsigned dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
        return era * 146097 + static_cast<int>(dayOfEra) - 719468;
    };

    return serial(end) - serial(start) + 1;
}

}  // namespace share
