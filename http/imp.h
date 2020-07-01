//
// Created by wangyu on 2020/6/14.
//

#ifndef WS_IMP_H
#define WS_IMP_H

#include "./comm.h"
#include <unordered_map>

namespace ws{
namespace http{

namespace imp{

// Response and Request

inline
bool has_header(const Headers &headers, const std::string &key) {
    return headers.find(key) != headers.end();
}

inline
const std::string& get_header_value(const Headers &headers, const std::string &key,
        size_t id = 0, const std::string& def = "") {
    auto range = headers.equal_range(key);
    auto it = range.first;
    std::advance(it, id);
    if(it != range.second){
        return it->second;
    }
    return def;
}

inline
size_t get_header_value_count(const Headers &headers, const std::string &key) {
    auto range = headers.equal_range(key);
    return std::distance(range.first, range.second);
}


std::string gmt_time_now();

inline std::string time_to_str(time_t t){
    struct tm timeinfo{};
    gmtime_r(&t, &timeinfo);
    char buffer[128];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %T GMT", &timeinfo);
    return buffer;
}

inline time_t str_to_time(const std::string& s){
    struct tm timeinfo{};
    strptime(s.c_str(), "%a, %d %b %Y %T GMT", &timeinfo);
    time_t t;
    localtime_r(&t, &timeinfo);
    return t;
}


const char *status_message(int status);

const char *find_content_type(const std::string &, const std::unordered_map<std::string, std::string> &);


} // end namespace imp


} // end namespace http
} // namespace ws

#endif //WS_IMP_H
