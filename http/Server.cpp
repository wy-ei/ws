//
// Created by wangyu on 2020/6/13.
//

#include <iostream>

#include "Server.h"
#include "Middleware.h"
#include "imp.h"
#include <cassert>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


Server::Server() {
    tcp_server_.set_connection_open_callback(std::bind(&Server::on_connection, this, _1));
    tcp_server_.set_connection_close_callback(std::bind(&Server::on_close, this, _1));
}


bool Server::listen(const std::string& host, unsigned short port) {
    return tcp_server_.listen(host, port);
}

void Server::use(const std::string &method, const std::string &pattern, const Handler& handler) {
    router_.use(method, pattern, handler);
}

void Server::use(const std::shared_ptr<Middleware>& mw) {
    middleware_.push_back(mw);
}

void Server::on_connection(SP<Conn> conn) {
    auto context = new Context(conn, router_, middleware_);
    std::shared_ptr<void> a(context, [](void* p){
        debug("close http- >\n");
        auto* ctx = static_cast<Context*>(p);
        delete ctx;
    });

    conn->context(a);
    conn->set_message_callback(std::bind(&Context::on_message, context, _1, _2));
}

void Server::on_close(const SP<Conn>& conn) {
   // auto* ctx = static_cast<Context*>(conn->context());
   // delete ctx;
}
