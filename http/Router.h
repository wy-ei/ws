//
// Created by wangyu on 2020/6/13.
//

#ifndef EX1_ROUTER_H
#define EX1_ROUTER_H

#include <functional>
#include <utility>
#include <vector>
#include <regex>
#include <unordered_map>

#include "http/comm.h"
#include "Request.h"
#include "Response.h"

namespace ws{
namespace http{
using namespace net;

class Router {
public:
    bool handle(Request& req, Response& res) const;
    void use(const std::string& method, const std::string& pattern, const Handler& handler);
private:
    static std::regex compile_pattern_to_regex(const std::string& pattern);

    struct Route{
        Route(std::regex re, Handler  handler):re_(std::move(re)), handler_(std::move(handler)){}
        std::regex re_;
        const Handler handler_;
    };

    std::unordered_map<std::string, std::vector<Route>> route_map_;
};

} // end namespace http
} // namespace ws

#endif //EX1_ROUTER_H
