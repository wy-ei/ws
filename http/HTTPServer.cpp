//
// Created by wangyu on 2020/6/13.
//

#include <iostream>

#include "HTTPServer.h"
#include "mw/Middleware.h"

namespace ws{
namespace http{
using namespace net;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::string;


bool HTTPServer::listen(const std::string& host, unsigned short port) {
    TCPServer server(thread_num_);
    server.set_connection_open_callback(std::bind(&HTTPServer::on_connection, this, _1));
    return server.listen(host, port);
}

void HTTPServer::use(const std::string &method, const std::string &pattern, const Handler& handler) {
    router_.use(method, pattern, handler);
}

void HTTPServer::use(const std::shared_ptr<Middleware>& mw) {
    middleware_.push_back(mw);
}

void HTTPServer::on_connection(std::shared_ptr<Conn> conn) {
    auto context = new Context(conn, router_, middleware_);
    conn->context(context);
    conn->set_message_callback(std::bind(&Context::on_message, context, _1, _2));
    conn->set_close_callback(std::bind(&HTTPServer::on_close, this, _1));
}

void HTTPServer::on_close(const std::shared_ptr<Conn>& conn) {
    LOG_DEBUG << "HTTP: on close";
    auto* ctx = static_cast<Context*>(conn->context());
    delete ctx;
}

} // end namespace http
} // namespace ws