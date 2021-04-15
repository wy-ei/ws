//
// Created by wangyu on 2020/6/13.
//

#include "Router.h"
#include "log/logging.h"
#include "utils/path.h"
#include "utils/str.h"

namespace ws{
namespace http{
using namespace net;

bool Router::handle(Request &req, Response &res) const {
    LOG_DEBUG << "dispatching " <<  req.method << " " << req.path;

    auto func = [&](const Route& route){
        std::cmatch m;
        bool matched = std::regex_match(req.path.c_str(), m, route.re_);
        if(!matched){
            return false;
        }
        route.handler_(req, res);
        return true;
    };

    if(route_map_.count(req.method) != 0){
        for(const Route& route: route_map_.at(req.method)){
            if(func(route)){
                return true;
            }
        }
    }
    if(route_map_.count("*") != 0){
        for(const Route& route: route_map_.at("*")){
            if(func(route)){
                return true;
            }
        }
    }

    return false;
}

void Router::use(const std::string &method, const std::string &pattern, const Handler &handler) {
    Route route(compile_pattern_to_regex(pattern), handler);
    route_map_[method].push_back(route);
}

std::regex Router::compile_pattern_to_regex(const std::string &pattern) {
    // path-to-regex

    std::string re;
    ws::str::split(pattern, '/', [&re](std::string s){
        if(s[0] == ':'){
            re += "/([^/]+)";
        }else{
            re += "/" + s;
        }
    });
    return std::regex(re);
}

} // end namespace http
} // namespace ws