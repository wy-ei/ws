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

inline uint64_t get_header_value_uint64(const Headers &headers, const std::string& key,
        uint64_t def = 0) {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return std::strtoull(it->second.data(), nullptr, 10);
    }
    return def;
}

inline
size_t get_header_value_count(const Headers &headers, const std::string &key) {
    auto range = headers.equal_range(key);
    return std::distance(range.first, range.second);
}

const char *status_message(int status);

} // end namespace imp


} // end namespace http
} // namespace ws

#endif //WS_IMP_H
