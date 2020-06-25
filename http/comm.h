//
// Created by dodo on 2020/6/25.
//

#ifndef WS_HTTP_COMM_H
#define WS_HTTP_COMM_H

#include <functional>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <unordered_map>

namespace ws {
namespace http {

class Request;
class Response;

using Params = std::unordered_map<std::string, std::string>;
using Handler = std::function<void(Request &, Response &)>;
using Headers = std::multimap<std::string, std::string>;


} // end namespace http
} // end namespace ws

#endif //WS_HTTP_COMM_H
