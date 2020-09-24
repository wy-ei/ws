//
// Created by 王钰 on 2020/8/10.
//
#include <memory>
#include "../net/TCPServer.h"

using namespace ws::net;
using namespace std::placeholders;

void message_handler(const std::shared_ptr<Conn>& conn, const char* message, size_t len){
    conn->write(message, len);
}

void connection_handler(std::shared_ptr<Conn> conn){
    conn->set_message_callback(std::bind(message_handler, conn, _1, _2));
}

int main(){
    TCPServer server(10);
    server.set_connection_open_callback(connection_handler);
    server.listen("localhost", 8002);
}