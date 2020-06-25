//
// Created by wangyu on 2020/6/13.
//

#include <iostream>

#include "HTTPServer.h"
#include "Middleware.h"
#include "imp.h"

namespace ws{
namespace http{
using namespace net;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::string;

#define SP std::shared_ptr

HTTPServer::HTTPServer() {
    tcp_server_.set_connection_open_callback(std::bind(&HTTPServer::on_connection, this, _1));
    tcp_server_.set_connection_close_callback(std::bind(&HTTPServer::on_close, this, _1));
}


bool HTTPServer::listen(const std::string& host, unsigned short port) {
    return tcp_server_.listen(host, port);
}

void HTTPServer::use(const std::string &method, const std::string &pattern, const Handler& handler) {
    router_.use(method, pattern, handler);
}

void HTTPServer::use(const std::shared_ptr<Middleware>& mw) {
    middleware_.push_back(mw);
}

void HTTPServer::on_connection(SP<Conn> conn) {
    auto context = new Context(conn, router_, middleware_);
    SP<void> a(context, [](void* p){
        LOG_DEBUG << "close http connection";
        auto* ctx = static_cast<Context*>(p);
        delete ctx;
    });

    conn->context(a);
    conn->set_message_callback(std::bind(&Context::on_message, context, _1, _2));
}

void HTTPServer::on_close(const SP<Conn>& conn) {
   // auto* ctx = static_cast<Context*>(conn->context());
   // delete ctx;
}

} // end namespace http
} // namespace ws