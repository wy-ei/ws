//
// Created by wangyu on 2020/6/13.
//

#ifndef WS_HTTPSERVER_H
#define WS_HTTPSERVER_H

#include <sys/socket.h>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <memory>
#include <list>

#include "../net/TCPServer.h"
#include "./comm.h"
#include "Middleware.h"
#include "Request.h"
#include "Response.h"
#include "Router.h"
#include "../net/Timeout.h"
#include "Context.h"

namespace ws{
namespace http{

using namespace net;

class HTTPServer {
public:
    HTTPServer();
    bool listen(const std::string& host, unsigned short port);
    void use(const std::string& method, const std::string& pattern, const Handler& handler);
    void use(const std::shared_ptr<Middleware>& mw);
private:
    void on_connection(std::shared_ptr<Conn> conn);
    void on_close(const std::shared_ptr<Conn>& conn);
    // void on_request();

    Router router_;

    TCPServer tcp_server_;

    size_t num_thread_ = 20;
    std::atomic<bool> running_ {false};
    Timeout timer {5000};

    std::unordered_map<int, Context*> connections;

    std::vector<std::shared_ptr<Middleware>> middleware_;
};

}
}

#endif //WS_HTTPSERVER_H
