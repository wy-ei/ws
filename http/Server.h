//
// Created by wangyu on 2020/6/13.
//

#ifndef WS_SERVER_H
#define WS_SERVER_H

#include <sys/socket.h>
#include <atomic>
#include <unordered_map>
#include <functional>
#include <memory>
#include <list>

#include "../net/TCPServer.h"

#include "../net/comm.h"
#include "Middleware.h"
#include "Request.h"
#include "Response.h"
#include "Router.h"
#include "../base/Timeout.h"
#include "Context.h"

class Server {
public:
    Server();
    bool listen(const std::string& host, unsigned short port);
    void use(const std::string& method, const std::string& pattern, const Handler& handler);
    void use(const std::shared_ptr<Middleware>& mw);
private:
    void on_connection(SP<Conn> conn);
    void on_close(const SP<Conn>& conn);
    // void on_request();

    Router router_;

    TCPServer tcp_server_;

    size_t num_thread_ = 20;
    std::atomic<bool> running_ {false};
    Timeout timer {5000};

    std::unordered_map<int, Context*> connections;

    std::vector<std::shared_ptr<Middleware>> middleware_;
};


#endif //WS_SERVER_H
