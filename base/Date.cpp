//
// Created by dodo on 2020/7/4.
//

#include "Date.h"
#include <ctime>

std::string Date::to_string(const std::string &fmt) const {
    struct tm time_info{};
    gmtime_r(&time_, &time_info);
    char buffer[50];
    strftime(buffer, sizeof(buffer), fmt.c_str(), &time_info);
    return buffer;
}

std::string Date::to_utc_string() const{
    // UTC: "Sat, 04 Jul 2020 08:39:59 GMT"
    return to_string("%a, %d %b %Y %T GMT");
}

std::string Date::to_locale_string(const std::string &fmt) const{
    struct tm time_info{};
    localtime_r(&time_, &time_info);
    char buffer[128];
    strftime(buffer, sizeof(buffer), fmt.c_str(), &time_info);
    return buffer;
}

std::string Date::to_locale_string() const {
    return to_locale_string("%F %X");
}

Date Date::from_utc_string(const std::string &s){
    struct tm time_info{};
    strptime(s.c_str(), "%a, %d %b %Y %T GMT", &time_info);
    time_t t = mktime(&time_info) - timezone;
    return Date(t);
}

Date Date::from_locale_time_string(const std::string &fmt, const std::string &s) {
    struct tm time_info{};
    strptime(s.c_str(), fmt.c_str(), &time_info);
    time_t t = mktime(&time_info);
    return Date(t);
}








